#include "History.h"
#include "../../common/Response.h"
#include <drogon/drogon.h>

namespace api
{
namespace v1
{

void History::getHistory(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto userId = req->attributes()->get<int64_t>("userId");
    auto db = drogon::app().getDbClient();

    Json::Value ret;
    Json::Value historyList(Json::arrayValue);

    auto rows = db->execSqlSync(
        "SELECT h.id, h.city_id, c.city_name, c.country_code, h.searched_at::text "
        "FROM search_history h "
        "JOIN weather_cities c ON h.city_id = c.id "
        "WHERE h.user_id = $1 "
        "ORDER BY h.searched_at DESC "
        "LIMIT 50",
        userId);

    for (const auto &row : rows)
    {
        Json::Value item;
        item["id"] = row["id"].as<int64_t>();
        item["city_id"] = row["city_id"].as<int64_t>();
        item["city_name"] = row["city_name"].as<std::string>();
        item["country_code"] = row["country_code"].as<std::string>();
        item["searched_at"] = row["searched_at"].as<std::string>();
        historyList.append(item);
    }

    ret["history"] = historyList;
    ret["count"] = historyList.size();
    callback(common::success(ret));
}

void History::clearHistory(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto userId = req->attributes()->get<int64_t>("userId");
    auto db = drogon::app().getDbClient();

    auto result = db->execSqlSync(
        "DELETE FROM search_history WHERE user_id = $1",
        userId);

    Json::Value ret;
    ret["message"] = "Search history cleared";
    callback(common::success(ret));
}

void History::addSearch(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto userId = req->attributes()->get<int64_t>("userId");
    auto jsonPtr = req->getJsonObject();
    if (!jsonPtr)
    {
        callback(common::badRequest("Request body must be JSON"));
        return;
    }

    auto &json = *jsonPtr;
    if (!json.isMember("city_name"))
    {
        callback(common::badRequest("city_name is required"));
        return;
    }

    auto cityName = json["city_name"].asString();
    auto countryCode = json.isMember("country_code") ? json["country_code"].asString() : "";

    auto db = drogon::app().getDbClient();

    // Find or create city
    auto cityRows = db->execSqlSync(
        "SELECT id FROM weather_cities WHERE city_name = $1 AND country_code = $2",
        cityName, countryCode);

    int64_t cityId = 0;
    if (cityRows.empty())
    {
        cityRows = db->execSqlSync(
            "SELECT id FROM weather_cities WHERE city_name = $1 LIMIT 1",
            cityName);
    }

    if (!cityRows.empty())
    {
        cityId = cityRows[0]["id"].as<int64_t>();
    }
    else
    {
        auto insertResult = db->execSqlSync(
            "INSERT INTO weather_cities (city_name, country_code) VALUES ($1, $2) RETURNING id",
            cityName, countryCode);
        cityId = insertResult[0]["id"].as<int64_t>();
    }

    auto result = db->execSqlSync(
        "INSERT INTO search_history (user_id, city_id) VALUES ($1, $2) RETURNING id",
        userId, cityId);

    Json::Value ret;
    ret["id"] = result[0]["id"].as<int64_t>();
    ret["city_id"] = cityId;
    ret["city_name"] = cityName;
    callback(common::created(ret, "Search recorded"));
}

} // namespace v1
} // namespace api
