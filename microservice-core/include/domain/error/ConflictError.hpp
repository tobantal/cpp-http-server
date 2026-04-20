#pragma once

#include "HttpError.hpp"

/**
 * @file ConflictError.hpp
 * @brief Conflict error (409)
 * @author Anton Tobolkin
 */

/**
 * @class ConflictError
 * @brief HTTP 409 Conflict error
 */
class ConflictError : public HttpError
{
public:
    /**
     * @brief Construct ConflictError
     * @param msg Error message
     */
    explicit ConflictError(const std::string &msg = "Conflict")
        : HttpError(409, msg) {}
};
