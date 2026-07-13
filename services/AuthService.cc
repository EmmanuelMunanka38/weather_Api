#include "AuthService.h"
#include "../common/Bcrypt.h"
#include "../common/JwtUtils.h"
#include "../common/Utils.h"
#include <drogon/drogon.h>

using namespace drogon::orm;
using namespace drogon;

drogon::orm::DbClientPtr AuthService::getDb()
{
    return app().getDbClient();
}

Json::Value AuthService::registerUser(const std::string &email,
                                      const std::string &username,
                                      const std::string &password)
{
    auto cleanEmail = common::trim(common::toLower(email));
    auto cleanUsername = common::trim(username);

    if (cleanEmail.empty() || cleanUsername.empty() || password.empty())
    {
        throw std::runtime_error("Email, username, and password are required");
    }

    if (!common::isValidEmail(cleanEmail))
    {
        throw std::runtime_error("Invalid email format");
    }

    if (password.size() < 6)
    {
        throw std::runtime_error("Password must be at least 6 characters");
    }

    auto db = getDb();

    // Check if email exists
    auto existing = db->execSqlSync(
        "SELECT id FROM users WHERE email = $1", cleanEmail);
    if (!existing.empty())
    {
        throw std::runtime_error("Email already registered");
    }

    std::string passwordHash = common::Bcrypt::hashPassword(password);

    // Insert user and get ID
    auto insertResult = db->execSqlSync(
        "INSERT INTO users (email, username, password_hash) VALUES ($1, $2, $3) RETURNING id",
        cleanEmail, cleanUsername, passwordHash);

    if (insertResult.empty())
    {
        throw std::runtime_error("Failed to create user");
    }

    int64_t userId = insertResult[0]["id"].as<int64_t>();

    Json::Value payload;
    payload["user_id"] = userId;
    payload["email"] = cleanEmail;
    payload["username"] = cleanUsername;

    Json::Value ret;
    ret["id"] = userId;
    ret["email"] = cleanEmail;
    ret["username"] = cleanUsername;
    ret["token"] = common::JwtUtils::encode(payload);
    return ret;
}

Json::Value AuthService::loginUser(const std::string &email,
                                   const std::string &password)
{
    auto cleanEmail = common::trim(common::toLower(email));
    auto db = getDb();

    Json::Value ret;

    auto rows = db->execSqlSync(
        "SELECT id, email, username, password_hash FROM users WHERE email = $1",
        cleanEmail);

    if (rows.empty())
    {
        ret["error"] = "Invalid email or password";
        return ret;
    }

    int64_t userId = rows[0]["id"].as<int64_t>();
    std::string storedHash = rows[0]["password_hash"].as<std::string>();
    std::string username = rows[0]["username"].as<std::string>();

    if (!common::Bcrypt::verifyPassword(password, storedHash))
    {
        ret["error"] = "Invalid email or password";
        return ret;
    }

    Json::Value payload;
    payload["user_id"] = userId;
    payload["email"] = cleanEmail;
    payload["username"] = username;

    ret["id"] = userId;
    ret["email"] = cleanEmail;
    ret["username"] = username;
    ret["token"] = common::JwtUtils::encode(payload);
    return ret;
}

Json::Value AuthService::getProfile(int64_t userId)
{
    auto db = getDb();
    Json::Value ret;

    auto rows = db->execSqlSync(
        "SELECT id, email, username, created_at FROM users WHERE id = $1",
        userId);

    if (rows.empty())
    {
        ret["error"] = "User not found";
        return ret;
    }

    ret["id"] = rows[0]["id"].as<int64_t>();
    ret["email"] = rows[0]["email"].as<std::string>();
    ret["username"] = rows[0]["username"].as<std::string>();
    ret["created_at"] = rows[0]["created_at"].as<std::string>();
    return ret;
}
