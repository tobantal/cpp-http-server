#pragma once

#include "HttpError.hpp"

class GatewayTimeoutError : public HttpError
{
public:
    explicit GatewayTimeoutError(const std::string &msg = "Gateway timeout")
        : HttpError(504, msg) {}
};