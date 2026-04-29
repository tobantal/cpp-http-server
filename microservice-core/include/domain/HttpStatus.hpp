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
int toInt(HttpStatus status) noexcept;

/**
 * @brief Get reason phrase for status code
 * @param code HTTP status code as integer
 * @return Reason phrase string
 */
std::string getReasonPhrase(int code);

/**
 * @brief Get reason phrase for HttpStatus enum
 * @param status HttpStatus enum value
 * @return Reason phrase string
 */
std::string getReasonPhrase(HttpStatus status);