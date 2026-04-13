#pragma once

#include <stdexcept>
#include <string>

class HttpError : public std::runtime_error
{
public:
    explicit HttpError(int statusCode, const std::string &message)
        : std::runtime_error(message), statusCode_(statusCode), message_(message) {}

    int statusCode() const noexcept { return statusCode_; }
    const std::string &message() const noexcept { return message_; }

protected:
    int statusCode_;
    std::string message_;
};