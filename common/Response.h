#pragma once

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include <string>

namespace common
{

inline drogon::HttpResponsePtr jsonResponse(drogon::HttpStatusCode code,
                                           const Json::Value &data)
{
    auto resp = drogon::HttpResponse::newHttpJsonResponse(data);
    resp->setStatusCode(code);
    resp->setContentTypeCode(drogon::CT_APPLICATION_JSON);
    return resp;
}

inline drogon::HttpResponsePtr success(const Json::Value &data,
                                       const std::string &message = "ok")
{
    Json::Value ret;
    ret["status"] = "success";
    ret["message"] = message;
    ret["data"] = data;
    return jsonResponse(drogon::k200OK, ret);
}

inline drogon::HttpResponsePtr created(const Json::Value &data,
                                       const std::string &message = "created")
{
    Json::Value ret;
    ret["status"] = "success";
    ret["message"] = message;
    ret["data"] = data;
    return jsonResponse(drogon::k201Created, ret);
}

inline drogon::HttpResponsePtr error(drogon::HttpStatusCode code,
                                     const std::string &message)
{
    Json::Value ret;
    ret["status"] = "error";
    ret["message"] = message;
    ret["code"] = static_cast<int>(code);
    return jsonResponse(code, ret);
}

inline drogon::HttpResponsePtr badRequest(const std::string &message = "Bad request")
{
    return error(drogon::k400BadRequest, message);
}

inline drogon::HttpResponsePtr unauthorized(const std::string &message = "Unauthorized")
{
    return error(drogon::k401Unauthorized, message);
}

inline drogon::HttpResponsePtr forbidden(const std::string &message = "Forbidden")
{
    return error(drogon::k403Forbidden, message);
}

inline drogon::HttpResponsePtr notFound(const std::string &message = "Not found")
{
    return error(drogon::k404NotFound, message);
}

inline drogon::HttpResponsePtr conflict(const std::string &message = "Conflict")
{
    return error(drogon::k409Conflict, message);
}

inline drogon::HttpResponsePtr serverError(const std::string &message = "Internal server error")
{
    return error(drogon::k500InternalServerError, message);
}

} // namespace common
