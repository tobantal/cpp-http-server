#pragma once

#include "HttpError.hpp"

class ForbiddenError : public HttpError
{
public:
    explicit ForbiddenError(const std::string &msg = "Forbidden")
        : HttpError(403, msg) {}
};