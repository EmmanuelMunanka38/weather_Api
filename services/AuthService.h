#pragma once

#include <drogon/HttpController.h>
#include <json/json.h>

class AuthService
{
  public:
    static Json::Value registerUser(const std::string &email,
                                    const std::string &username,
                                    const std::string &password);

    static Json::Value loginUser(const std::string &email,
                                 const std::string &password);

    static Json::Value getProfile(int64_t userId);

  private:
    static drogon::orm::DbClientPtr getDb();
};
