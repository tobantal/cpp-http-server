#pragma once

#include "HttpError.hpp"

/**
 * @file RequestTimeoutError.hpp
 * @brief Request timeout error (408)
 * @author Anton Tobolkin
 */

/**
 * @class RequestTimeoutError
 * @brief HTTP 408 Request Timeout error
 */
class RequestTimeoutError : public HttpError
{
public:
    /**
     * @brief Construct RequestTimeoutError
     * @param msg Error message
     */
    explicit RequestTimeoutError(const std::string &msg = "Request timeout")
        : HttpError(408, msg) {}
};
