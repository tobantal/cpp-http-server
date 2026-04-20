#pragma once

#include "HttpError.hpp"

/**
 * @file UnauthorizedError.hpp
 * @brief Unauthorized error (401)
 * @author Anton Tobolkin
 */

/**
 * @class UnauthorizedError
 * @brief HTTP 401 Unauthorized error
 */
class UnauthorizedError : public HttpError
{
public:
    /**
     * @brief Construct UnauthorizedError
     * @param msg Error message
     */
    explicit UnauthorizedError(const std::string &msg = "Unauthorized")
        : HttpError(401, msg) {}
};
