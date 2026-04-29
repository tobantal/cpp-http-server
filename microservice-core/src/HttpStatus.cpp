/**
 * @file HttpStatus.cpp
 * @brief HTTP status utilities implementation
 * @author Anton Tobolkin
 */

#include "domain/HttpStatus.hpp"

int toInt(HttpStatus status) noexcept
{
    return static_cast<int>(status);
}

std::string getReasonPhrase(int code)
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

std::string getReasonPhrase(HttpStatus status)
{
    return getReasonPhrase(toInt(status));
}
