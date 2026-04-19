#pragma once

#include "HttpError.hpp"

class UnauthorizedError : public HttpError
{
public:
    explicit UnauthorizedError(const std::string &msg = "Unauthorized")
        : HttpError(401, msg) {}
};