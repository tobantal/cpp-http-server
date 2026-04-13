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

    void setBody(const std::string& body) override
    {
        res_.body() = body;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        res_.set(name, value);
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

private:
    boost::beast::http::response<boost::beast::http::string_body>& res_;
};