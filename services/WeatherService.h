#pragma once

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

class WeatherService
{
  public:
    static Json::Value getCurrentWeather(const std::string &city);
    static Json::Value getForecast(const std::string &city, int days);
    static Json::Value searchCities(const std::string &query);

  private:
    static drogon::orm::DbClientPtr getDb();
    static Json::Value weatherToJson(const struct WeatherRecord &record);
};
