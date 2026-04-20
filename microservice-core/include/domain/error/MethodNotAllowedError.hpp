#pragma once

#include "HttpError.hpp"

/**
 * @file MethodNotAllowedError.hpp
 * @brief Method not allowed error (405)
 * @author Anton Tobolkin
 */

/**
 * @class MethodNotAllowedError
 * @brief HTTP 405 Method Not Allowed error
 */
class MethodNotAllowedError : public HttpError
{
public:
    /**
     * @brief Construct MethodNotAllowedError
     * @param msg Error message
     */
    explicit MethodNotAllowedError(const std::string &msg = "Method not allowed")
        : HttpError(405, msg) {}
};
