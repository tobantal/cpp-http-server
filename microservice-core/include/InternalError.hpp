#pragma once

#include "HttpError.hpp"

class InternalError : public HttpError
{
public:
    explicit InternalError(const std::string &msg = "Internal server error")
        : HttpError(500, msg) {}
};