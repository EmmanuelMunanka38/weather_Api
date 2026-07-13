#include "JwtAuthFilter.h"
#include "../common/JwtUtils.h"
#include "../common/Response.h"

void JwtAuthFilter::doFilter(const drogon::HttpRequestPtr &req,
                             drogon::FilterCallback &&fcb,
                             drogon::FilterChainCallback &&fccb)
{
    auto authHeader = req->getHeader("Authorization");
    if (authHeader.empty() || authHeader.substr(0, 7) != "Bearer ")
    {
        fcb(common::unauthorized("Missing or invalid Authorization header"));
        return;
    }

    std::string token = authHeader.substr(7);
    try
    {
        auto payload = common::JwtUtils::decode(token);
        auto userId = payload["user_id"].asInt64();
        auto username = payload["username"].asString();

        req->attributes()->insert("userId", userId);
        req->attributes()->insert("username", username);

        fccb();
    }
    catch (const std::exception &e)
    {
        fcb(common::unauthorized(std::string("Invalid token: ") + e.what()));
    }
}
