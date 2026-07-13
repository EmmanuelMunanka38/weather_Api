#pragma once

#include <drogon/HttpController.h>

namespace api
{
namespace v1
{

class Weather : public drogon::HttpController<Weather>
{
  public:
    METHOD_LIST_BEGIN
    METHOD_ADD(Weather::getCurrentWeather, "/current/{1}", Get, "JwtAuthFilter");
    METHOD_ADD(Weather::getForecast, "/forecast/{1}", Get, "JwtAuthFilter");
    METHOD_ADD(Weather::searchCities, "/search", Get, "JwtAuthFilter");
    METHOD_LIST_END

    void getCurrentWeather(const drogon::HttpRequestPtr &req,
                           std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                           std::string &&city);

    void getForecast(const drogon::HttpRequestPtr &req,
                     std::function<void(const drogon::HttpResponsePtr &)> &&callback,
                     std::string &&city);

    void searchCities(const drogon::HttpRequestPtr &req,
                      std::function<void(const drogon::HttpResponsePtr &)> &&callback);
};

} // namespace v1
} // namespace api
