#pragma once

#include "HttpError.hpp"

class NotFoundError : public HttpError
{
public:
    explicit NotFoundError(const std::string &msg = "Not found")
        : HttpError(404, msg) {}
};