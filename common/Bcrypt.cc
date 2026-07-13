#include "Bcrypt.h"
#include <openssl/sha.h>
#include <openssl/rand.h>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace common
{

static const std::string base64Chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string base64Encode(const unsigned char *data, size_t len)
{
    std::string result;
    int i = 0;
    int j = 0;
    unsigned char charArray3[3];
    unsigned char charArray4[4];

    while (len--)
    {
        charArray3[i++] = *(data++);
        if (i == 3)
        {
            charArray4[0] = (charArray3[0] & 0xfc) >> 2;
            charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
            charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);
            charArray4[3] = charArray3[2] & 0x3f;

            for (i = 0; i < 4; i++)
                result += base64Chars[charArray4[i]];
            i = 0;
        }
    }
    if (i)
    {
        for (j = i; j < 3; j++)
            charArray3[j] = '\0';

        charArray4[0] = (charArray3[0] & 0xfc) >> 2;
        charArray4[1] = ((charArray3[0] & 0x03) << 4) + ((charArray3[1] & 0xf0) >> 4);
        charArray4[2] = ((charArray3[1] & 0x0f) << 2) + ((charArray3[2] & 0xc0) >> 6);

        for (j = 0; j < i + 1; j++)
            result += base64Chars[charArray4[j]];

        while (i++ < 3)
            result += '=';
    }
    return result;
}

static std::string pbkdf2Sha256(const std::string &password,
                                const std::string &salt,
                                int iterations,
                                int keyLength)
{
    unsigned char *result = new unsigned char[keyLength];
    PKCS5_PBKDF2_HMAC(password.c_str(), static_cast<int>(password.size()),
                       reinterpret_cast<const unsigned char *>(salt.c_str()),
                       static_cast<int>(salt.size()),
                       iterations, EVP_sha256(), keyLength, result);

    std::string output(reinterpret_cast<char *>(result), keyLength);
    delete[] result;
    return output;
}

std::string Bcrypt::hashPassword(const std::string &password)
{
    unsigned char saltRaw[16];
    if (RAND_bytes(saltRaw, 16) != 1)
        throw std::runtime_error("Failed to generate salt");

    std::string salt = base64Encode(saltRaw, 16);
    std::string hash = pbkdf2Sha256(password, salt, 100000, 32);
    std::string encodedHash = base64Encode(reinterpret_cast<const unsigned char *>(hash.c_str()), hash.size());

    return "pbkdf2_sha256$" + salt + "$" + encodedHash;
}

bool Bcrypt::verifyPassword(const std::string &password, const std::string &storedHash)
{
    // Parse: pbkdf2_sha256$salt$hash
    auto firstDollar = storedHash.find('$');
    auto secondDollar = storedHash.find('$', firstDollar + 1);

    if (firstDollar == std::string::npos || secondDollar == std::string::npos)
        return false;

    std::string salt = storedHash.substr(firstDollar + 1, secondDollar - firstDollar - 1);
    std::string expectedHash = storedHash.substr(secondDollar + 1);

    std::string computedHash = pbkdf2Sha256(password, salt, 100000, 32);
    std::string encodedComputed = base64Encode(
        reinterpret_cast<const unsigned char *>(computedHash.c_str()), computedHash.size());

    return encodedComputed == expectedHash;
}

} // namespace common
