#pragma once

#include "HttpError.hpp"

/**
 * @file ForbiddenError.hpp
 * @brief Forbidden error (403)
 * @author Anton Tobolkin
 */

/**
 * @class ForbiddenError
 * @brief HTTP 403 Forbidden error
 */
class ForbiddenError : public HttpError
{
public:
    /**
     * @brief Construct ForbiddenError
     * @param msg Error message
     */
    explicit ForbiddenError(const std::string &msg = "Forbidden")
        : HttpError(403, msg) {}
};
