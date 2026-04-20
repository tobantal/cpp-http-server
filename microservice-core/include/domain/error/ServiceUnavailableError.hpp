#pragma once

#include "HttpError.hpp"

/**
 * @file ServiceUnavailableError.hpp
 * @brief Service unavailable error (503)
 * @author Anton Tobolkin
 */

/**
 * @class ServiceUnavailableError
 * @brief HTTP 503 Service Unavailable error
 */
class ServiceUnavailableError : public HttpError
{
public:
    /**
     * @brief Construct ServiceUnavailableError
     * @param msg Error message
     */
    explicit ServiceUnavailableError(const std::string &msg = "Service unavailable")
        : HttpError(503, msg) {}
};
