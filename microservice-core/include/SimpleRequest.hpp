#pragma once

#include "IRequest.hpp"
#include "StringUtils.hpp"
#include "PathParamExtractor.hpp"
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <cctype>

struct SimpleRequest : IRequest
{
    SimpleRequest(const std::string& method,
                  const std::string& path,
                  const std::string& body,
                  const std::string& ip,
                  int port,
                  const std::map<std::string, std::string>& headers = {})
        : method_(method), path_(path), body_(body), ip_(ip), port_(port), headers_(headers)
    {
    }

    SimpleRequest()
        : method_("GET"), path_("/"), body_(""), ip_("127.0.0.1"), port_(80)
    {
    }

    std::string getPath() const override { return path_; }

    std::vector<std::string> getPathSegments() const override
    {
        std::vector<std::string> segments;
        std::string segment;

        for (char ch : path_)
        {
            if (ch == '/')
            {
                if (!segment.empty())
                {
                    segments.push_back(segment);
                    segment.clear();
                }
            }
            else if (ch == '?')
            {
                break;
            }
            else
            {
                segment += ch;
            }
        }

        if (!segment.empty())
        {
            segments.push_back(segment);
        }

        return segments;
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
        return PathParamExtractor::getByIndex(path_, pathPattern_, index);
    }

    std::map<std::string, std::string> getQueryParams() const override
    {
        return queryParams_;
    }

    std::optional<std::string> getQueryParam(const std::string& name) const override
    {
        auto it = queryParams_.find(name);
        if (it != queryParams_.end())
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

    std::string getBody() const override { return body_; }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    std::string getMethod() const override { return method_; }

    std::string getIp() const override { return ip_; }
    int getPort() const override { return port_; }

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

    void setMethod(const std::string& method) { method_ = method; }
    void setPath(const std::string& path) { path_ = path; }
    void setIp(const std::string& ip) { ip_ = ip; }
    void setPort(int port) { port_ = port; }

private:
    std::string method_;
    std::string path_;
    std::string body_;
    std::string ip_;
    int port_;
    std::string pathPattern_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> queryParams_;
    std::map<std::string, std::string> attributes_;
};