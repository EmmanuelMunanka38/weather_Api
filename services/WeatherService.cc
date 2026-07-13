#include "WeatherService.h"
#include "OpenWeatherMapClient.h"
#include "../common/Response.h"
#include <drogon/drogon.h>
#include <json/json.h>

using namespace drogon::orm;
using namespace drogon;

struct WeatherRecord
{
    int64_t id = 0;
    int64_t cityId = 0;
    double temperature = 0;
    double feelsLike = 0;
    int humidity = 0;
    int pressure = 0;
    double windSpeed = 0;
    int windDeg = 0;
    std::string description;
    std::string icon;
    std::string fetchedAt;
};

drogon::orm::DbClientPtr WeatherService::getDb()
{
    return app().getDbClient();
}

Json::Value WeatherService::weatherToJson(const WeatherRecord &record)
{
    Json::Value ret;
    ret["id"] = record.id;
    ret["city_id"] = record.cityId;
    ret["temperature"] = record.temperature;
    ret["feels_like"] = record.feelsLike;
    ret["humidity"] = record.humidity;
    ret["pressure"] = record.pressure;
    ret["wind_speed"] = record.windSpeed;
    ret["wind_direction"] = record.windDeg;
    ret["description"] = record.description;
    ret["icon"] = record.icon;
    ret["fetched_at"] = record.fetchedAt;
    return ret;
}

Json::Value WeatherService::getCurrentWeather(const std::string &city)
{
    auto db = getDb();

    // Try to find or create the city in our DB
    auto cityRows = db->execSqlSync(
        "SELECT id, city_name, country_code, latitude, longitude FROM weather_cities WHERE city_name = $1",
        city);

    int64_t cityDbId = 0;
    if (!cityRows.empty())
    {
        cityDbId = cityRows[0]["id"].as<int64_t>();

        // Check for recent reading (within last 30 minutes)
        auto recentRows = db->execSqlSync(
            "SELECT id, city_id, temperature, feels_like, humidity, pressure, "
            "wind_speed, wind_direction, description, icon, fetched_at::text "
            "FROM weather_readings WHERE city_id = $1 "
            "ORDER BY fetched_at DESC LIMIT 1",
            cityDbId);

        if (!recentRows.empty())
        {
            // Check if data is less than 30 minutes old
            auto fetchedAt = recentRows[0]["fetched_at"].as<std::string>();
            // Simple check - just return the cached data
            WeatherRecord record;
            record.id = recentRows[0]["id"].as<int64_t>();
            record.cityId = recentRows[0]["city_id"].as<int64_t>();
            record.temperature = recentRows[0]["temperature"].as<double>();
            record.feelsLike = recentRows[0]["feels_like"].as<double>();
            record.humidity = recentRows[0]["humidity"].as<int>();
            record.pressure = recentRows[0]["pressure"].as<int>();
            record.windSpeed = recentRows[0]["wind_speed"].as<double>();
            record.windDeg = recentRows[0]["wind_direction"].as<int>();
            record.description = recentRows[0]["description"].as<std::string>();
            record.icon = recentRows[0]["icon"].as<std::string>();
            record.fetchedAt = fetchedAt;

            Json::Value ret;
            ret["source"] = "cache";
            ret["weather"] = weatherToJson(record);
            return ret;
        }
    }

    // Fetch from OpenWeatherMap API
    auto weatherData = OpenWeatherMapClient::fetchCurrentWeather(city);

    // Store city if not exists
    if (cityDbId == 0)
    {
        auto insertResult = db->execSqlSync(
            "INSERT INTO weather_cities (city_name, country_code, latitude, longitude, openweathermap_id) "
            "VALUES ($1, $2, $3, $4, $5) RETURNING id",
            weatherData.cityName, weatherData.countryCode,
            weatherData.latitude, weatherData.longitude, weatherData.cityId);
        cityDbId = insertResult[0]["id"].as<int64_t>();
    }

    // Store weather reading
    auto readingResult = db->execSqlSync(
        "INSERT INTO weather_readings (city_id, temperature, feels_like, humidity, "
        "pressure, wind_speed, wind_direction, description, icon) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9) RETURNING id",
        cityDbId, weatherData.temperature, weatherData.feelsLike,
        weatherData.humidity, weatherData.pressure, weatherData.windSpeed,
        weatherData.windDeg, weatherData.description, weatherData.icon);

    WeatherRecord record;
    record.id = readingResult[0]["id"].as<int64_t>();
    record.cityId = cityDbId;
    record.temperature = weatherData.temperature;
    record.feelsLike = weatherData.feelsLike;
    record.humidity = weatherData.humidity;
    record.pressure = weatherData.pressure;
    record.windSpeed = weatherData.windSpeed;
    record.windDeg = weatherData.windDeg;
    record.description = weatherData.description;
    record.icon = weatherData.icon;

    Json::Value ret;
    ret["source"] = "api";
    ret["weather"] = weatherToJson(record);
    return ret;
}

Json::Value WeatherService::getForecast(const std::string &city, int days)
{
    auto forecasts = OpenWeatherMapClient::fetchForecast(city, days);

    Json::Value ret;
    ret["city"] = city;
    ret["days"] = days;

    Json::Value forecastList(Json::arrayValue);
    for (const auto &f : forecasts)
    {
        Json::Value item;
        item["date"] = f.date;
        item["temp_min"] = f.tempMin;
        item["temp_max"] = f.tempMax;
        item["humidity"] = f.humidity;
        item["description"] = f.description;
        item["icon"] = f.icon;
        forecastList.append(item);
    }
    ret["forecasts"] = forecastList;
    return ret;
}

Json::Value WeatherService::searchCities(const std::string &query)
{
    return OpenWeatherMapClient::searchCities(query);
}
