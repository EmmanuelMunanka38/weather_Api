#pragma once

#include <string>
#include <json/json.h>

class OpenWeatherMapClient
{
  public:
    struct WeatherData
    {
        double temperature = 0;
        double feelsLike = 0;
        int humidity = 0;
        int pressure = 0;
        double windSpeed = 0;
        int windDeg = 0;
        std::string description;
        std::string icon;
        std::string cityName;
        std::string countryCode;
        double latitude = 0;
        double longitude = 0;
        int cityId = 0;
    };

    struct ForecastDay
    {
        std::string date;
        double tempMin = 0;
        double tempMax = 0;
        int humidity = 0;
        std::string description;
        std::string icon;
    };

    static void setApiKey(const std::string &key);
    static WeatherData fetchCurrentWeather(const std::string &city);
    static std::vector<ForecastDay> fetchForecast(const std::string &city, int days = 5);
    static Json::Value searchCities(const std::string &query);

  private:
    static std::string apiKey_;
    static std::string httpGet(const std::string &url);
    static Json::Value parseJson(const std::string &json);
};
