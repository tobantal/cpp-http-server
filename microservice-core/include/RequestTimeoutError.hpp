#pragma once

#include "HttpError.hpp"

class RequestTimeoutError : public HttpError
{
public:
    explicit RequestTimeoutError(const std::string &msg = "Request timeout")
        : HttpError(408, msg) {}
};