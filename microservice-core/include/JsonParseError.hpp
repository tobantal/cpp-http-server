#pragma once

#include "HttpError.hpp"

class JsonParseError : public HttpError
{
public:
    explicit JsonParseError(const std::string &msg = "Invalid JSON")
        : HttpError(400, msg) {}
};