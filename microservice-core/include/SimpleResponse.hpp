#pragma once

#include "IResponse.hpp"
#include "StringUtils.hpp"
#include <string>
#include <map>
#include <optional>

class SimpleResponse : public IResponse
{
public:
    SimpleResponse(int status = 200, const std::string& body = "")
        : status_(status), body_(body)
    {
    }

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
        std::string nameLower = StringUtils::toLower(name);
        for (const auto& [key, value] : headers_)
        {
            if (StringUtils::toLower(key) == nameLower)
            {
                return value;
            }
        }
        return std::nullopt;
    }

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
};