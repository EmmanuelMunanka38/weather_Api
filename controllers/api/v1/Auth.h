#pragma once

#include <drogon/HttpController.h>

namespace api
{
namespace v1
{

class Auth : public drogon::HttpController<Auth>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Auth::registerUser, "/register", Post);
    METHOD_ADD(Auth::login, "/login", Post);
    METHOD_ADD(Auth::getProfile, "/profile", Get, "JwtAuthFilter");
    METHOD_LIST_END

    void registerUser(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void login(const drogon::HttpRequestPtr &req,
               std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void getProfile(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace v1
} // namespace api
