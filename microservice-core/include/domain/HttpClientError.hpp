#pragma once

#include <string>
#include <cstdint>

/**
 * @file HttpClientError.hpp
 * @brief HTTP client error codes and result
 * @author Anton Tobolkin
 */

/**
 * @enum HttpClientError
 * @brief HTTP client error types
 */
enum class HttpClientError : uint8_t
{
    None,
    DnsFailed,
    ConnectTimeout,
    ConnectionRefused,
    WriteTimeout,
    ReadTimeout,
    UnknownError
};

/**
 * @struct HttpClientResult
 * @brief Result of an HTTP client request
 */
struct HttpClientResult
{
    HttpClientError error = HttpClientError::None;
    std::string errorMessage;

    /**
     * @brief Check if the request was successful
     * @return true if no error occurred
     */
    bool ok() const { return error == HttpClientError::None; }
};

/**
 * @brief Convert HttpClientError to string
 * @param e Error code
 * @return String representation of the error
 */
inline std::string httpClientErrorToString(HttpClientError e)
{
    switch (e)
    {
    case HttpClientError::None: return "none";
    case HttpClientError::DnsFailed: return "dns_failed";
    case HttpClientError::ConnectTimeout: return "connect_timeout";
    case HttpClientError::ConnectionRefused: return "connection_refused";
    case HttpClientError::WriteTimeout: return "write_timeout";
    case HttpClientError::ReadTimeout: return "read_timeout";
    case HttpClientError::UnknownError: return "unknown_error";
    default: return "unknown";
    }
}
