#include "Auth.h"
#include "../../services/AuthService.h"
#include "../../common/Response.h"

namespace api
{
namespace v1
{

void Auth::registerUser(const drogon::HttpRequestPtr &req,
                         std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr)
    {
        callback(common::badRequest("Request body must be JSON"));
        return;
    }

    auto &json = *jsonPtr;
    if (!json.isMember("email") || !json.isMember("username") || !json.isMember("password"))
    {
        callback(common::badRequest("email, username, and password are required"));
        return;
    }

    try
    {
        auto result = AuthService::registerUser(
            json["email"].asString(),
            json["username"].asString(),
            json["password"].asString());
        callback(common::created(result, "User registered successfully"));
    }
    catch (const std::exception &e)
    {
        callback(common::conflict(e.what()));
    }
}

void Auth::login(const drogon::HttpRequestPtr &req,
                  std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr)
    {
        callback(common::badRequest("Request body must be JSON"));
        return;
    }

    auto &json = *jsonPtr;
    if (!json.isMember("email") || !json.isMember("password"))
    {
        callback(common::badRequest("email and password are required"));
        return;
    }

    auto result = AuthService::loginUser(
        json["email"].asString(),
        json["password"].asString());

    if (result.isMember("error"))
    {
        callback(common::unauthorized(result["error"].asString()));
        return;
    }

    callback(common::success(result, "Login successful"));
}

void Auth::getProfile(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto userId = req->attributes()->get<int64_t>("userId");

    auto result = AuthService::getProfile(userId);
    if (result.isMember("error"))
    {
        callback(common::notFound(result["error"].asString()));
        return;
    }

    callback(common::success(result));
}

} // namespace v1
} // namespace api
