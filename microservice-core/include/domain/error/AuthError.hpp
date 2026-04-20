#pragma once

#include "HttpError.hpp"

/**
 * @file AuthError.hpp
 * @brief Authentication error (401)
 * @author Anton Tobolkin
 */

/**
 * @class AuthError
 * @brief HTTP 401 Authentication error
 */
class AuthError : public HttpError
{
public:
    /**
     * @brief Construct AuthError
     * @param msg Error message
     */
    explicit AuthError(const std::string &msg = "Authentication error")
        : HttpError(401, msg) {}
};
