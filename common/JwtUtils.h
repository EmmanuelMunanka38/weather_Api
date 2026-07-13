#pragma once

#include <string>
#include <json/json.h>

namespace common
{

class JwtUtils
{
  public:
    static void setSecret(const std::string &secret);
    static std::string getSecret();

    static std::string encode(const Json::Value &payload, int expireSeconds = 86400);
    static Json::Value decode(const std::string &token);
    static bool verify(const std::string &token);

  private:
    static std::string secret_;
    static std::string base64Encode(const std::string &data);
    static std::string base64Decode(const std::string &data);
    static std::string hmacSha256(const std::string &key, const std::string &data);
};

} // namespace common
