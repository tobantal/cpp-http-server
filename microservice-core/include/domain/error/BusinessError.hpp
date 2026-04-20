#pragma once

#include "HttpError.hpp"

/**
 * @file BusinessError.hpp
 * @brief Business logic error (400)
 * @author Anton Tobolkin
 */

/**
 * @class BusinessError
 * @brief HTTP 400 Business error
 */
class BusinessError : public HttpError
{
public:
    /**
     * @brief Construct BusinessError
     * @param msg Error message
     */
    explicit BusinessError(const std::string &msg = "Business error")
        : HttpError(400, msg) {}
};
