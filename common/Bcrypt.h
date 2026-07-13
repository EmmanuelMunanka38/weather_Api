#pragma once

#include <string>

namespace common
{

class Bcrypt
{
  public:
    static std::string hashPassword(const std::string &password);
    static bool verifyPassword(const std::string &password, const std::string &hash);
};

} // namespace common
