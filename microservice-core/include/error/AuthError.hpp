#pragma once

#include "HttpError.hpp"

class AuthError : public HttpError
{
public:
    explicit AuthError(const std::string &msg = "Authentication error")
        : HttpError(401, msg) {}
};