#pragma once

#include "HttpError.hpp"

/**
 * @file InternalError.hpp
 * @brief Internal server error (500)
 * @author Anton Tobolkin
 */

/**
 * @class InternalError
 * @brief HTTP 500 Internal Server Error
 */
class InternalError : public HttpError
{
public:
    /**
     * @brief Construct InternalError
     * @param msg Error message
     */
    explicit InternalError(const std::string &msg = "Internal server error")
        : HttpError(500, msg) {}
};
