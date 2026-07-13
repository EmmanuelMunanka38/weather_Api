#pragma once

#include <drogon/HttpController.h>

namespace api
{
namespace v1
{

class History : public drogon::HttpController<History>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(History::getHistory, "", Get, "JwtAuthFilter");
    METHOD_ADD(History::clearHistory, "", Delete, "JwtAuthFilter");
    METHOD_ADD(History::addSearch, "", Post, "JwtAuthFilter");
    METHOD_LIST_END

    void getHistory(const drogon::HttpRequestPtr &req,
                    std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void clearHistory(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void addSearch(const drogon::HttpRequestPtr &req,
                   std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace v1
} // namespace api
