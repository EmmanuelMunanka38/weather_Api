#include "Favorites.h"
#include "../../common/Response.h"
#include <drogon/drogon.h>

namespace api
{
namespace v1
{

void Favorites::listFavorites(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback)
{
    auto userId = req->attributes()->get<int64_t>("userId");
    auto db = drogon::app().getDbClient();

    Json::Value ret;
    Json::Value favoritesList(Json::arrayValue);

    auto rows = db->execSqlSync(
        "SELECT f.id, f.city_id, c.city_name, c.country_code, c.latitude, c.longitude "
        "FROM user_favorites f "
        "JOIN weather_cities c ON f.city_id = c.id "
        "WHERE f.user_id = $1 "
        "ORDER BY f.created_at DESC",
        userId);

    for (const auto &row : rows)
    {
        Json::Value fav;
        fav["id"] = row["id"].as<int64_t>();
        fav["city_id"] = row["city_id"].as<int64_t>();
        fav["city_name"] = row["city_name"].as<std::string>();
        fav["country_code"] = row["country_code"].as<std::string>();
        fav["latitude"] = row["latitude"].as<double>();
        fav["longitude"] = row["longitude"].as<double>();
        favoritesList.append(fav);
    }

    ret["favorites"] = favoritesList;
    ret["count"] = favoritesList.size();
    callback(common::success(ret));
}

void Favorites::addFavorite(
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
        // Try without country code
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
        // Insert city with placeholder data
        auto insertResult = db->execSqlSync(
            "INSERT INTO weather_cities (city_name, country_code) VALUES ($1, $2) RETURNING id",
            cityName, countryCode);
        cityId = insertResult[0]["id"].as<int64_t>();
    }

    // Check if already favorited
    auto existing = db->execSqlSync(
        "SELECT id FROM user_favorites WHERE user_id = $1 AND city_id = $2",
        userId, cityId);

    if (!existing.empty())
    {
        callback(common::conflict("City already in favorites"));
        return;
    }

    auto result = db->execSqlSync(
        "INSERT INTO user_favorites (user_id, city_id) VALUES ($1, $2) RETURNING id",
        userId, cityId);

    Json::Value ret;
    ret["id"] = result[0]["id"].as<int64_t>();
    ret["city_id"] = cityId;
    ret["city_name"] = cityName;
    ret["country_code"] = countryCode;

    callback(common::created(ret, "Added to favorites"));
}

void Favorites::removeFavorite(
    const drogon::HttpRequestPtr &req,
    std::function<void(const drogon::HttpResponsePtr &)> &&callback,
    int64_t cityId)
{
    auto userId = req->attributes()->get<int64_t>("userId");
    auto db = drogon::app().getDbClient();

    auto result = db->execSqlSync(
        "DELETE FROM user_favorites WHERE user_id = $1 AND city_id = $2 RETURNING id",
        userId, cityId);

    if (result.empty())
    {
        callback(common::notFound("Favorite not found"));
        return;
    }

    Json::Value ret;
    ret["message"] = "Removed from favorites";
    callback(common::success(ret));
}

} // namespace v1
} // namespace api
