#pragma once

#include "IResponse.hpp"
#include "StringUtils.hpp"
#include <boost/beast/http.hpp>
#include <string>
#include <map>
#include <optional>

struct BeastResponseAdapter : IResponse
{
    BeastResponseAdapter(boost::beast::http::response<boost::beast::http::string_body>& res)
        : res_(res) {}

    void setStatus(int code) override
    {
        res_.result(boost::beast::http::status(code));
    }

    void setStatus(HttpStatus status) override
    {
        res_.result(boost::beast::http::status(toInt(status)));
    }

    void setBody(const std::string& body) override
    {
        res_.body() = body;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        res_.set(name, value);
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
        res_.set(boost::beast::http::field::set_cookie, cookie);
    }

    int getStatus() const override
    {
        return res_.result_int();
    }

    std::string getBody() const override
    {
        return res_.body();
    }

    std::map<std::string, std::string> getHeaders() const override
    {
        std::map<std::string, std::string> headers;

        for (auto const& field : res_)
        {
            std::string name = std::string(field.name_string());
            std::string value = std::string(field.value());
            headers[name] = value;
        }

        return headers;
    }

    std::optional<std::string> getHeader(const std::string& name) const override
    {
        std::string nameLower = StringUtils::toLower(name);

        for (auto const& field : res_)
        {
            std::string fieldName = std::string(field.name_string());
            if (StringUtils::toLower(fieldName) == nameLower)
            {
                return std::string(field.value());
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

    /**
     * @brief Установить X-Trace-ID в ответ
     * @param id Trace ID
     */
    void setTraceId(const std::string& id) override
    {
        setHeader("X-Trace-ID", id);
    }

private:
    boost::beast::http::response<boost::beast::http::string_body>& res_;
};