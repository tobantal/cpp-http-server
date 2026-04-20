#pragma once

#include "HttpError.hpp"

/**
 * @file BadRequestError.hpp
 * @brief Bad request error (400)
 * @author Anton Tobolkin
 */

/**
 * @class BadRequestError
 * @brief HTTP 400 Bad Request error
 */
class BadRequestError : public HttpError
{
public:
    /**
     * @brief Construct BadRequestError
     * @param msg Error message
     */
    explicit BadRequestError(const std::string &msg = "Bad request")
        : HttpError(400, msg) {}
};
