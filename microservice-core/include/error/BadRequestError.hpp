#pragma once

#include "HttpError.hpp"

class BadRequestError : public HttpError
{
public:
    explicit BadRequestError(const std::string &msg = "Bad request")
        : HttpError(400, msg) {}
};