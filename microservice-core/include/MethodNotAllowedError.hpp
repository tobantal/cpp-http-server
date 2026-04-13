#pragma once

#include "HttpError.hpp"

class MethodNotAllowedError : public HttpError
{
public:
    explicit MethodNotAllowedError(const std::string &msg = "Method not allowed")
        : HttpError(405, msg) {}
};