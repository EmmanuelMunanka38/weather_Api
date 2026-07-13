#include "Weather.h"
#include "../../services/WeatherService.h"
#include "../../common/Response.h"
#include <cstdlib>

namespace api
{
namespace v1
{

void Weather::getCurrentWeather(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    std::string &&city)
{
    if (city.empty())
    {
        callback(common::badRequest("City name is required"));
        return;
    }

    try
    {
        auto result = WeatherService::getCurrentWeather(city);
        callback(common::success(result));
    }
    catch (const std::exception &e)
    {
        callback(common::error(drogon::k502BadGateway,
                               std::string("Failed to fetch weather: ") + e.what()));
    }
}

void Weather::getForecast(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    std::string &&city)
{
    if (city.empty())
    {
        callback(common::badRequest("City name is required"));
        return;
    }

    int days = 5;
    auto daysStr = req->getParameter("days");
    if (!daysStr.empty())
    {
        try
        {
            days = std::stoi(daysStr);
            if (days < 1 || days > 5)
                days = 5;
        }
        catch (...)
        {
            days = 5;
        }
    }

    try
    {
        auto result = WeatherService::getForecast(city, days);
        callback(common::success(result));
    }
    catch (const std::exception &e)
    {
        callback(common::error(drogon::k502BadGateway,
                               std::string("Failed to fetch forecast: ") + e.what()));
    }
}

void Weather::searchCities(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto query = req->getParameter("q");
    if (query.empty())
    {
        callback(common::badRequest("Search query 'q' is required"));
        return;
    }

    try
    {
        auto result = WeatherService::searchCities(query);
        callback(common::success(result));
    }
    catch (const std::exception &e)
    {
        callback(common::error(drogon::k502BadGateway,
                               std::string("Failed to search cities: ") + e.what()));
    }
}

} // namespace v1
} // namespace api
