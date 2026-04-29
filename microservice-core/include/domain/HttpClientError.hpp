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
    /** @brief Error code (default: None) */
    HttpClientError error = HttpClientError::None;

    /** @brief Human-readable error message */
    std::string errorMessage;

    /**
     * @brief Check if the request was successful
     * @return true if no error occurred
     */
    bool ok() const;
};

/**
 * @brief Convert HttpClientError to string
 * @param e Error code
 * @return String representation of the error
 */
std::string httpClientErrorToString(HttpClientError e);