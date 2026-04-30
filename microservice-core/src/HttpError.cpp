#include "domain/error/HttpError.hpp"

int HttpError::statusCode() const noexcept {
    return statusCode_;
}

const std::string& HttpError::message() const noexcept {
    return message_;
}