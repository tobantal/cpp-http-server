#pragma once

#include "HttpError.hpp"

/**
 * @file NotFoundError.hpp
 * @brief Not found error (404)
 * @author Anton Tobolkin
 */

/**
 * @class NotFoundError
 * @brief HTTP 404 Not Found error
 */
class NotFoundError : public HttpError
{
public:
    /**
     * @brief Construct NotFoundError
     * @param msg Error message
     */
    explicit NotFoundError(const std::string &msg = "Not found")
        : HttpError(404, msg) {}
};
