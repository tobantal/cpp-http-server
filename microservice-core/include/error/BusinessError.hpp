#pragma once

#include "HttpError.hpp"

class BusinessError : public HttpError
{
public:
    explicit BusinessError(const std::string &msg = "Business error")
        : HttpError(400, msg) {}
};