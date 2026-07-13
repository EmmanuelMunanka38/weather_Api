#pragma once

#include <drogon/HttpController.h>

namespace api
{
namespace v1
{

class Favorites : public drogon::HttpController<Favorites>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Favorites::listFavorites, "", Get, "JwtAuthFilter");
    METHOD_ADD(Favorites::addFavorite, "", Post, "JwtAuthFilter");
    METHOD_ADD(Favorites::removeFavorite, "/{1}", Delete, "JwtAuthFilter");
    METHOD_LIST_END

    void listFavorites(const drogon::HttpRequestPtr &req,
                       std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void addFavorite(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    void removeFavorite(const drogon::HttpRequestPtr &req,
                        std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                        int64_t cityId);
};

} // namespace v1
} // namespace api
