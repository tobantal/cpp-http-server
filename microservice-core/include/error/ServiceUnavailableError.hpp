#pragma once

#include "HttpError.hpp"

class ServiceUnavailableError : public HttpError
{
public:
    explicit ServiceUnavailableError(const std::string &msg = "Service unavailable")
        : HttpError(503, msg) {}
};