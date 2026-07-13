#include "JwtUtils.h"
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <json/json.h>
#include <ctime>
#include <sstream>
#include <stdexcept>

namespace common
{

std::string JwtUtils::secret_ = "weather_app_secret_key_2024";

void JwtUtils::setSecret(const std::string &secret)
{
    secret_ = secret;
}

std::string JwtUtils::getSecret()
{
    return secret_;
}

std::string JwtUtils::base64Encode(const std::string &data)
{
    BIO *bio, *b64;
    BUF_MEM *bufferPtr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(bio, data.c_str(), static_cast<int>(data.size()));
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &bufferPtr);

    std::string result(bufferPtr->data, bufferPtr->length);
    BIO_free_all(bio);

    for (auto &c : result)
    {
        if (c == '+')
            c = '-';
        else if (c == '/')
            c = '_';
    }
    while (!result.empty() && result.back() == '=')
        result.pop_back();

    return result;
}

std::string JwtUtils::base64Decode(const std::string &data)
{
    std::string padded = data;
    while (padded.size() % 4 != 0)
        padded += '=';

    for (auto &c : padded)
    {
        if (c == '-')
            c = '+';
        else if (c == '_')
            c = '/';
    }

    BIO *bio, *b64;
    int decodeLen = static_cast<int>(padded.size());
    char *buffer = new char[decodeLen + 1];

    bio = BIO_new_mem_buf(padded.c_str(), -1);
    b64 = BIO_new(BIO_f_base64());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
    int len = BIO_read(bio, buffer, decodeLen);
    buffer[len] = '\0';

    std::string result(buffer, len);
    delete[] buffer;
    BIO_free_all(bio);
    return result;
}

std::string JwtUtils::hmacSha256(const std::string &key, const std::string &data)
{
    unsigned int resultLen = 0;
    unsigned char result[EVP_MAX_MD_SIZE];

    HMAC(EVP_sha256(),
         key.c_str(), static_cast<int>(key.size()),
         reinterpret_cast<const unsigned char *>(data.c_str()), data.size(),
         result, &resultLen);

    return std::string(reinterpret_cast<char *>(result), resultLen);
}

std::string JwtUtils::encode(const Json::Value &payload, int expireSeconds)
{
    Json::Value header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";

    Json::Value fullPayload = payload;
    Json::Int64 now = static_cast<Json::Int64>(std::time(nullptr));
    fullPayload["iat"] = now;
    fullPayload["exp"] = now + static_cast<Json::Int64>(expireSeconds);

    Json::StreamWriterBuilder writer;
    std::string headerStr = Json::writeString(writer, header);
    std::string payloadStr = Json::writeString(writer, fullPayload);

    std::string encodedHeader = base64Encode(headerStr);
    std::string encodedPayload = base64Encode(payloadStr);

    std::string signatureInput = encodedHeader + "." + encodedPayload;
    std::string signature = hmacSha256(secret_, signatureInput);
    std::string encodedSignature = base64Encode(signature);

    return encodedHeader + "." + encodedPayload + "." + encodedSignature;
}

Json::Value JwtUtils::decode(const std::string &token)
{
    // Split token into 3 parts
    std::vector<std::string> parts;
    std::istringstream iss(token);
    std::string part;
    while (std::getline(iss, part, '.'))
    {
        parts.push_back(part);
    }
    if (parts.size() != 3)
        throw std::runtime_error("Invalid token format");

    std::string signatureInput = parts[0] + "." + parts[1];
    std::string expectedSig = base64Encode(hmacSha256(secret_, signatureInput));

    if (parts[2] != expectedSig)
        throw std::runtime_error("Invalid token signature");

    std::string payloadJson = base64Decode(parts[1]);
    Json::Value payload;
    Json::CharReaderBuilder reader;
    std::istringstream stream(payloadJson);
    std::string errors;
    if (!Json::parseFromStream(reader, stream, &payload, &errors))
        throw std::runtime_error("Invalid token payload: " + errors);

    if (payload.isMember("exp"))
    {
        Json::Int64 exp = payload["exp"].asLargestInt();
        Json::Int64 nowTs = static_cast<Json::Int64>(std::time(nullptr));
        if (nowTs > exp)
            throw std::runtime_error("Token expired");
    }

    return payload;
}

bool JwtUtils::verify(const std::string &token)
{
    try
    {
        decode(token);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace common
