#pragma once

#include "HttpError.hpp"
#include <string>

/**
 * @file ConvertError.hpp
 * @brief Error for JSON parsing and conversion failures
 * @author Anton Tobolkin
 */

/**
 * @class ConvertError
 * @brief HTTP 400 error for JSON parsing failures and type conversion errors
 */
class ConvertError : public HttpError
{
public:
    /**
     * @brief Construct ConvertError with message
     * @param message Error description
     */
    explicit ConvertError(const std::string& message)
        : HttpError(400, message)
    {
    }

    /**
     * @brief Construct ConvertError with message and cause
     * @param message Error description
     * @param cause Underlying error reason
     */
    ConvertError(const std::string& message, const std::string& cause)
        : HttpError(400, message + ": " + cause)
    {
    }
};