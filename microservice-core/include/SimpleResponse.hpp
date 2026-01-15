#pragma once

#include "IResponse.hpp"
#include <string>
#include <map>
#include <optional>
#include <algorithm>
#include <cctype>

/**
 * @file SimpleResponse.hpp
 * @brief Простая реализация IResponse
 * @version 2.0
 * @author Anton Tobolkin
 */
class SimpleResponse : public IResponse
{
public:
    SimpleResponse(int status = 200, const std::string& body = "")
        : status_(status), body_(body)
    {
    }

    // =========================================================================
    // SETTERS
    // =========================================================================

    void setStatus(int code) override
    {
        status_ = code;
    }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        headers_[name] = value;
    }

    // =========================================================================
    // GETTERS
    // =========================================================================

    int getStatus() const override
    {
        return status_;
    }

    std::string getBody() const override
    {
        return body_;
    }

    std::map<std::string, std::string> getHeaders() const override
    {
        return headers_;
    }

    std::optional<std::string> getHeader(const std::string& name) const override
    {
        std::string nameLower = toLower(name);
        for (const auto& [key, value] : headers_) {
            if (toLower(key) == nameLower) {
                return value;
            }
        }
        return std::nullopt;
    }

    // =========================================================================
    // CONVENIENCE METHODS
    // =========================================================================

    void setResult(int code, 
                   const std::string& contentType, 
                   const std::string& body) override
    {
        setStatus(code);
        setHeader("Content-Type", contentType);
        setBody(body);
    }

private:
    int status_;
    std::string body_;
    std::map<std::string, std::string> headers_;

    static std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }
};
