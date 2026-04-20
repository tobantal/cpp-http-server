#pragma once

#include <string>

/**
 * @file HttpStatus.hpp
 * @brief HTTP status codes and reason phrases
 * @author Anton Tobolkin
 */

/**
 * @enum HttpStatus
 * @brief HTTP status codes as enum
 */
enum class HttpStatus : int
{
    Ok = 200,
    Created = 201,
    NoContent = 204,
    BadRequest = 400,
    Unauthorized = 401,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    Conflict = 409,
    PayloadTooLarge = 413,
    UnprocessableEntity = 422,
    InternalServerError = 500,
    ServiceUnavailable = 503,
    GatewayTimeout = 504
};

/**
 * @brief Convert HttpStatus to integer
 * @param status HttpStatus enum value
 * @return Integer status code
 */
inline int toInt(HttpStatus status) noexcept
{
    return static_cast<int>(status);
}

/**
 * @brief Get reason phrase for status code
 * @param code HTTP status code as integer
 * @return Reason phrase string
 */
inline std::string getReasonPhrase(int code)
{
    switch (code)
    {
    case 200: return "OK";
    case 201: return "Created";
    case 204: return "No Content";
    case 400: return "Bad Request";
    case 401: return "Unauthorized";
    case 403: return "Forbidden";
    case 404: return "Not Found";
    case 405: return "Method Not Allowed";
    case 409: return "Conflict";
    case 413: return "Payload Too Large";
    case 422: return "Unprocessable Entity";
    case 500: return "Internal Server Error";
    case 503: return "Service Unavailable";
    case 504: return "Gateway Timeout";
    default: return "Unknown";
    }
}

/**
 * @brief Get reason phrase for HttpStatus enum
 * @param status HttpStatus enum value
 * @return Reason phrase string
 */
inline std::string getReasonPhrase(HttpStatus status)
{
    return getReasonPhrase(toInt(status));
}
