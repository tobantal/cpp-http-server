#pragma once

#include "IResponse.hpp"
#include "StringUtils.hpp"
#include <string>
#include <map>
#include <optional>
#include <sstream>

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

    void setStatus(HttpStatus status) override
    {
        status_ = toInt(status);
    }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        headers_[name] = value;
    }

    void setCookie(const std::string& name,
                    const std::string& value,
                    const std::string& path = "/",
                    bool httpOnly = true,
                    bool secure = false,
                    int maxAge = -1) override
    {
        std::string cookie = name + "=" + value;
        if (!path.empty())
        {
            cookie += "; Path=" + path;
        }
        if (maxAge >= 0)
        {
            cookie += "; Max-Age=" + std::to_string(maxAge);
        }
        if (httpOnly)
        {
            cookie += "; HttpOnly";
        }
        if (secure)
        {
            cookie += "; Secure";
        }
        headers_["Set-Cookie"] = cookie;
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
        for (const auto& [key, val] : headers_)
        {
            if (StringUtils::toLower(key) == nameLower)
            {
                return val;
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

    void setResult(HttpStatus status,
                   const std::string& contentType,
                   const std::string& body) override
    {
        setResult(toInt(status), contentType, body);
    }

    void setTraceId(const std::string& id) override
    {
        setHeader("X-Trace-ID", id);
    }

private:
    int status_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};