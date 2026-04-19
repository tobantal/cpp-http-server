#pragma once

#include "HttpError.hpp"

class ConflictError : public HttpError
{
public:
    explicit ConflictError(const std::string &msg = "Conflict")
        : HttpError(409, msg) {}
};