#include "OpenWeatherMapClient.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <sstream>
#include <thread>
#include <chrono>

std::string OpenWeatherMapClient::apiKey_ = "YOUR_API_KEY_HERE";

void OpenWeatherMapClient::setApiKey(const std::string &key)
{
    apiKey_ = key;
}

std::string OpenWeatherMapClient::httpGet(const std::string &url)
{
    std::string host;
    std::string path;
    uint16_t port = 80;

    std::string cleanUrl = url;
    bool isHttps = false;
    if (cleanUrl.substr(0, 8) == "https://")
    {
        isHttps = true;
        cleanUrl = cleanUrl.substr(8);
    }
    else if (cleanUrl.substr(0, 7) == "http://")
    {
        cleanUrl = cleanUrl.substr(7);
    }

    auto slashPos = cleanUrl.find('/');
    if (slashPos != std::string::npos)
    {
        host = cleanUrl.substr(0, slashPos);
        path = cleanUrl.substr(slashPos);
    }
    else
    {
        host = cleanUrl;
        path = "/";
    }

    auto colonPos = host.find(':');
    if (colonPos != std::string::npos)
    {
        port = static_cast<uint16_t>(std::stoi(host.substr(colonPos + 1)));
        host = host.substr(0, colonPos);
    }

    if (isHttps)
        port = 443;

    std::string responseBody;
    bool requestDone = false;

    auto client = drogon::HttpClient::newHttpClient(
        (isHttps ? "https://" : "http://") + host + ":" + std::to_string(port));

    auto req = drogon::HttpRequest::newHttpRequest();
    req->setPath(path);
    req->setMethod(drogon::Get);
    req->addHeader("Host", host);
    req->addHeader("User-Agent", "WeatherApp/1.0");

    client->sendRequest(
        req,
        [&responseBody, &requestDone](drogon::ReqResult reqResult,
                                      const drogon::HttpResponsePtr &resp) {
            if (reqResult == drogon::ReqResult::Ok && resp)
            {
                responseBody = resp->getBody();
            }
            requestDone = true;
        });

    int maxWait = 3000;
    int waited = 0;
    while (!requestDone && waited < maxWait)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        waited += 100;
    }

    return responseBody;
}

Json::Value OpenWeatherMapClient::parseJson(const std::string &json)
{
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream stream(json);
    std::string errors;
    Json::parseFromStream(builder, stream, &root, &errors);
    return root;
}

OpenWeatherMapClient::WeatherData OpenWeatherMapClient::fetchCurrentWeather(const std::string &city)
{
    std::string url = "http://api.openweathermap.org/data/2.5/weather?q=" +
                      city + "&appid=" + apiKey_ + "&units=metric";

    std::string response = httpGet(url);
    Json::Value data = parseJson(response);

    WeatherData weather;
    if (data.isMember("cod") && data["cod"].asInt() != 200)
    {
        throw std::runtime_error(data.isMember("message") ? data["message"].asString() : "API error");
    }

    weather.temperature = data["main"]["temp"].asDouble();
    weather.feelsLike = data["main"]["feels_like"].asDouble();
    weather.humidity = data["main"]["humidity"].asInt();
    weather.pressure = data["main"]["pressure"].asInt();
    weather.windSpeed = data["wind"]["speed"].asDouble();
    weather.windDeg = data["wind"]["deg"].asInt();
    weather.description = data["weather"][0]["description"].asString();
    weather.icon = data["weather"][0]["icon"].asString();
    weather.cityName = data["name"].asString();
    weather.countryCode = data["sys"]["country"].asString();
    weather.latitude = data["coord"]["lat"].asDouble();
    weather.longitude = data["coord"]["lon"].asDouble();
    weather.cityId = data["id"].asInt();

    return weather;
}

std::vector<OpenWeatherMapClient::ForecastDay> OpenWeatherMapClient::fetchForecast(
    const std::string &city, int days)
{
    std::string url = "http://api.openweathermap.org/data/2.5/forecast?q=" +
                      city + "&appid=" + apiKey_ + "&units=metric&cnt=" + std::to_string(days * 8);

    std::string response = httpGet(url);
    Json::Value data = parseJson(response);

    std::vector<ForecastDay> forecasts;
    if (data.isMember("cod") && data["cod"].asInt() != 200)
    {
        throw std::runtime_error(data.isMember("message") ? data["message"].asString() : "API error");
    }

    if (data.isMember("list"))
    {
        for (const auto &item : data["list"])
        {
            ForecastDay day;
            day.date = item["dt_txt"].asString();
            day.tempMin = item["main"]["temp_min"].asDouble();
            day.tempMax = item["main"]["temp_max"].asDouble();
            day.humidity = item["main"]["humidity"].asInt();
            day.description = item["weather"][0]["description"].asString();
            day.icon = item["weather"][0]["icon"].asString();
            forecasts.push_back(day);
        }
    }

    return forecasts;
}

Json::Value OpenWeatherMapClient::searchCities(const std::string &query)
{
    std::string url = "http://api.openweathermap.org/geo/1.0/direct?q=" +
                      query + "&limit=5&appid=" + apiKey_;

    std::string response = httpGet(url);
    Json::Value data = parseJson(response);

    Json::Value results(Json::arrayValue);
    if (data.isArray())
    {
        for (const auto &item : data)
        {
            Json::Value city;
            city["name"] = item["name"].asString();
            city["country"] = item["country"].asString();
            city["latitude"] = item["lat"].asDouble();
            city["longitude"] = item["lon"].asDouble();
            city["state"] = item.isMember("state") ? item["state"].asString() : "";
            results.append(city);
        }
    }

    return results;
}
