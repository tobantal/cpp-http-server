#pragma once

#include <stdexcept>
#include <string>

/**
 * @file HttpError.hpp
 * @brief Base HTTP error class
 * @author Anton Tobolkin
 */

/**
 * @class HttpError
 * @brief HTTP error with status code and message
 */
class HttpError : public std::runtime_error
{
public:
    /**
     * @brief Construct HTTP error
     * @param statusCode HTTP status code
     * @param message Error message
     */
    explicit HttpError(int statusCode, const std::string &message)
        : std::runtime_error(message), statusCode_(statusCode), message_(message) {}

    /**
     * @brief Get HTTP status code
     * @return Status code
     */
    int statusCode() const noexcept;

    /**
     * @brief Get error message
     * @return Error message
     */
    const std::string &message() const noexcept;

protected:
    int statusCode_;
    std::string message_;
};