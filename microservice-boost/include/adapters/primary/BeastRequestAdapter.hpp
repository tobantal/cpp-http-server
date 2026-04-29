#pragma once

#include "domain/IRequest.hpp"
#include "util/IIdGenerator.hpp"
#include "util/Uuid7Generator.hpp"
#include "util/PathParamExtractor.hpp"
#include "ports/output/IEnvironment.hpp"
#include <boost/beast/http.hpp>
#include <map>
#include <string>
#include <vector>
#include <optional>

/**
 * @file BeastRequestAdapter.hpp
 * @brief Boost.Beast request adapter implementing IRequest
 * @author Anton Tobolkin
 */

/**
 * @struct BeastRequestAdapter
 * @brief IRequest implementation that wraps a Boost.Beast HTTP request
 */
struct BeastRequestAdapter : IRequest
{
    BeastRequestAdapter(
        const boost::beast::http::request<boost::beast::http::string_body>& req,
        const std::string& clientIp,
        int port = 80,
        std::shared_ptr<IIdGenerator> idGenerator = std::make_shared<Uuid7Generator>())
        : req_(req), ip_(clientIp), port_(port), body_(req.body()), idGenerator_(std::move(idGenerator)) {}

    std::string getPath() const override
    {
        auto target = std::string(req_.target());
        auto pos = target.find('?');
        std::string path = pos == std::string::npos ? target : target.substr(0, pos);
        return StringUtils::urlDecode(path);
    }

    std::vector<std::string> getPathSegments() const override
    {
        return StringUtils::splitPath(getPath());
    }

    std::string getPathPattern() const override
    {
        return pathPattern_;
    }

    void setPathPattern(const std::string& pattern) override
    {
        pathPattern_ = pattern;
    }

    std::optional<std::string> getPathParam(size_t index) const override
    {
        return PathParamExtractor::getByIndex(getPath(), pathPattern_, index);
    }

    std::map<std::string, std::string> getQueryParams() const override
    {
        if (cachedQueryParams_.has_value())
        {
            return cachedQueryParams_.value();
        }

        std::map<std::string, std::string> params = queryParams_;

        auto target = std::string(req_.target());
        auto pos = target.find('?');
        if (pos == std::string::npos)
        {
            cachedQueryParams_ = params;
            return params;
        }

        std::string query = target.substr(pos + 1);
        size_t start = 0;
        while (start < query.size())
        {
            auto eq = query.find('=', start);
            auto amp = query.find('&', start);
            if (eq == std::string::npos)
                break;

            std::string key = StringUtils::urlDecode(query.substr(start, eq - start));
            std::string value = amp == std::string::npos
                                    ? StringUtils::urlDecode(query.substr(eq + 1))
                                    : StringUtils::urlDecode(query.substr(eq + 1, amp - eq - 1));

            if (params.find(key) == params.end())
            {
                params[key] = value;
            }
            if (amp == std::string::npos)
                break;
            start = amp + 1;
        }
        cachedQueryParams_ = params;
        return params;
    }

    std::optional<std::string> getQueryParam(const std::string& name) const override
    {
        auto params = getQueryParams();
        auto it = params.find(name);
        if (it != params.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void setQueryParam(const std::string& name, const std::string& value) override
    {
        queryParams_[name] = value;
    }

    std::map<std::string, std::string> getParams() const override
    {
        return getQueryParams();
    }

    std::map<std::string, std::string> getHeaders() const override
    {
        std::map<std::string, std::string> headers = headers_;

        for (auto const& field : req_)
        {
            std::string name = std::string(field.name_string());
            std::string value = std::string(field.value());
            if (headers.find(name) == headers.end())
            {
                headers[name] = value;
            }
        }

        return headers;
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

        for (auto const& field : req_)
        {
            std::string fieldName = std::string(field.name_string());
            if (StringUtils::toLower(fieldName) == nameLower)
            {
                return std::string(field.value());
            }
        }

        return std::nullopt;
    }

    void setHeader(const std::string& name, const std::string& value) override
    {
        headers_[name] = value;
    }

    void setHeaders(const std::map<std::string, std::string>& headers) override
    {
        for (const auto& [name, value] : headers)
        {
            headers_[name] = value;
        }
    }

    std::string getBody() const override
    {
        return body_;
    }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    std::string getMethod() const override
    {
        return std::string(req_.method_string());
    }

    std::string getIp() const override
    {
        return ip_;
    }

    int getPort() const override
    {
        return port_;
    }

    std::optional<std::string> getBearerToken() const override
    {
        auto auth = getHeader("Authorization");
        if (!auth)
        {
            return std::nullopt;
        }

        const std::string bearerPrefix = "Bearer ";
        if (auth->length() > bearerPrefix.length() &&
            auth->substr(0, bearerPrefix.length()) == bearerPrefix)
        {
            return auth->substr(bearerPrefix.length());
        }

        return std::nullopt;
    }

    bool isJson() const override
    {
        auto contentType = getContentType();
        return contentType.find("json") != std::string::npos;
    }

    std::string getContentType() const override
    {
        auto ct = getHeader("Content-Type");
        return ct.value_or("");
    }

    void setAttribute(const std::string& name, const std::string& value) override
    {
        attributes_[name] = value;
    }

    std::optional<std::string> getAttribute(const std::string& name) const override
    {
        auto it = attributes_.find(name);
        if (it != attributes_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    void setObject(const std::string& name, std::shared_ptr<IEnvironment> obj) override
    {
        objects_[name] = obj;
    }

    std::optional<std::shared_ptr<IEnvironment>> getObject(const std::string& name) const override
    {
        auto it = objects_.find(name);
        if (it != objects_.end())
        {
            return it->second;
        }
        return std::nullopt;
    }

    std::string getTraceId() override
    {
        auto header = getHeader("X-Trace-ID");
        if (header)
        {
            return *header;
        }
        std::string id = idGenerator_->generate();
        setHeader("X-Trace-ID", id);
        return id;
    }

    void setTraceId(const std::string& id) override
    {
        setHeader("X-Trace-ID", id);
    }

private:
    const boost::beast::http::request<boost::beast::http::string_body>& req_;  // NOLINT(cppcoreguidelines-avoid-const-or-ref-data-members)
    std::string ip_;
    int port_;
    std::string body_;
    std::string pathPattern_;
    std::map<std::string, std::string> queryParams_;
    mutable std::optional<std::map<std::string, std::string>> cachedQueryParams_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> attributes_;
    std::map<std::string, std::shared_ptr<IEnvironment>> objects_;
    std::shared_ptr<IIdGenerator> idGenerator_;
};
