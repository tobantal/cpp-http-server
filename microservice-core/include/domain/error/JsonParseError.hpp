#pragma once

#include "HttpError.hpp"

/**
 * @file JsonParseError.hpp
 * @brief JSON parse error (400)
 * @author Anton Tobolkin
 */

/**
 * @class JsonParseError
 * @brief HTTP 400 JSON parse error
 */
class JsonParseError : public HttpError
{
public:
    /**
     * @brief Construct JsonParseError
     * @param msg Error message
     */
    explicit JsonParseError(const std::string &msg = "Invalid JSON")
        : HttpError(400, msg) {}
};
