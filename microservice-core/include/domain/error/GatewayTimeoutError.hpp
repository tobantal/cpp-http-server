#pragma once

#include "HttpError.hpp"

/**
 * @file GatewayTimeoutError.hpp
 * @brief Gateway timeout error (504)
 * @author Anton Tobolkin
 */

/**
 * @class GatewayTimeoutError
 * @brief HTTP 504 Gateway Timeout error
 */
class GatewayTimeoutError : public HttpError
{
public:
    /**
     * @brief Construct GatewayTimeoutError
     * @param msg Error message
     */
    explicit GatewayTimeoutError(const std::string &msg = "Gateway timeout")
        : HttpError(504, msg) {}
};
