// BeastRequestAdapter.hpp
#pragma once
#include "IRequest.hpp"
#include <boost/beast/http.hpp>
#include <map>
#include <string>
#include <vector>
#include <optional>
#include <algorithm>
#include <cctype>

/**
 * @file BeastRequestAdapter.hpp
 * @brief Адаптер для Boost.Beast HTTP запроса
 * @version 2.0
 * @author Anton Tobolkin
 */
struct BeastRequestAdapter : IRequest
{
    BeastRequestAdapter(
        const boost::beast::http::request<boost::beast::http::string_body>& req,
        const std::string& clientIp)
        : req_(req), ip_(clientIp), body_(req.body()) {}

    // =========================================================================
    // PATH
    // =========================================================================

    std::string getPath() const override
    {
        auto target = std::string(req_.target());
        auto pos = target.find('?');
        return pos == std::string::npos ? target : target.substr(0, pos);
    }

    std::vector<std::string> getPathSegments() const override
    {
        std::string path = getPath();
        std::vector<std::string> segments;
        std::string segment;
        
        for (char ch : path) {
            if (ch == '/') {
                if (!segment.empty()) {
                    segments.push_back(segment);
                    segment.clear();
                }
            } else {
                segment += ch;
            }
        }
        
        if (!segment.empty()) {
            segments.push_back(segment);
        }
        
        return segments;
    }

    // =========================================================================
    // PATH PARAMETERS
    // =========================================================================

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
        if (pathPattern_.empty()) {
            return std::nullopt;
        }
        
        auto pathSegments = getPathSegments();
        auto patternSegments = splitPath(pathPattern_);
        
        size_t wildcardIndex = 0;
        for (size_t i = 0; i < patternSegments.size() && i < pathSegments.size(); ++i) {
            if (patternSegments[i] == "*") {
                if (wildcardIndex == index) {
                    return pathSegments[i];
                }
                ++wildcardIndex;
            }
        }
        
        return std::nullopt;
    }

    // =========================================================================
    // QUERY PARAMETERS
    // =========================================================================

    std::map<std::string, std::string> getQueryParams() const override
    {
        std::map<std::string, std::string> params = queryParams_;
        
        // Парсим из URL
        auto target = std::string(req_.target());
        auto pos = target.find('?');
        if (pos == std::string::npos)
            return params;

        std::string query = target.substr(pos + 1);
        size_t start = 0;
        while (start < query.size())
        {
            auto eq = query.find('=', start);
            auto amp = query.find('&', start);
            if (eq == std::string::npos)
                break;

            std::string key = query.substr(start, eq - start);
            std::string value = amp == std::string::npos
                                    ? query.substr(eq + 1)
                                    : query.substr(eq + 1, amp - eq - 1);

            // Не перезаписываем установленные вручную параметры
            if (params.find(key) == params.end()) {
                params[key] = value;
            }
            if (amp == std::string::npos)
                break;
            start = amp + 1;
        }
        return params;
    }

    std::optional<std::string> getQueryParam(const std::string& name) const override
    {
        auto params = getQueryParams();
        auto it = params.find(name);
        if (it != params.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    void setQueryParam(const std::string& name, const std::string& value) override
    {
        queryParams_[name] = value;
    }

    // Deprecated alias
    std::map<std::string, std::string> getParams() const override
    {
        return getQueryParams();
    }

    // =========================================================================
    // HEADERS
    // =========================================================================

    std::map<std::string, std::string> getHeaders() const override
    {
        std::map<std::string, std::string> headers = headers_;
        
        for (auto const& field : req_)
        {
            std::string name = std::string(field.name_string());
            std::string value = std::string(field.value());
            // Не перезаписываем установленные вручную
            if (headers.find(name) == headers.end()) {
                headers[name] = value;
            }
        }
        
        return headers;
    }

    std::optional<std::string> getHeader(const std::string& name) const override
    {
        // Сначала проверяем установленные вручную (case-insensitive)
        std::string nameLower = toLower(name);
        for (const auto& [key, value] : headers_) {
            if (toLower(key) == nameLower) {
                return value;
            }
        }
        
        // Затем в оригинальном запросе (case-insensitive)
        for (auto const& field : req_)
        {
            std::string fieldName = std::string(field.name_string());
            if (toLower(fieldName) == nameLower) {
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
        for (const auto& [name, value] : headers) {
            headers_[name] = value;
        }
    }

    // =========================================================================
    // BODY
    // =========================================================================

    std::string getBody() const override
    {
        return body_;
    }

    void setBody(const std::string& body) override
    {
        body_ = body;
    }

    // =========================================================================
    // METHOD
    // =========================================================================

    std::string getMethod() const override
    {
        return std::string(req_.method_string());
    }

    // =========================================================================
    // CONNECTION INFO
    // =========================================================================

    std::string getIp() const override
    {
        return ip_;
    }

    int getPort() const override
    {
        return 80;
    }

    // =========================================================================
    // CONVENIENCE METHODS
    // =========================================================================

    std::optional<std::string> getBearerToken() const override
    {
        auto auth = getHeader("Authorization");
        if (!auth) {
            return std::nullopt;
        }
        
        const std::string bearerPrefix = "Bearer ";
        if (auth->length() > bearerPrefix.length() &&
            auth->substr(0, bearerPrefix.length()) == bearerPrefix) {
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

    // =========================================================================
    // ATTRIBUTES
    // =========================================================================

    void setAttribute(const std::string& name, const std::string& value) override
    {
        attributes_[name] = value;
    }

    std::optional<std::string> getAttribute(const std::string& name) const override
    {
        auto it = attributes_.find(name);
        if (it != attributes_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

private:
    const boost::beast::http::request<boost::beast::http::string_body>& req_;
    std::string ip_;
    std::string body_;
    std::string pathPattern_;
    std::map<std::string, std::string> queryParams_;
    std::map<std::string, std::string> headers_;
    std::map<std::string, std::string> attributes_;

    static std::string toLower(const std::string& str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static std::vector<std::string> splitPath(const std::string& path)
    {
        std::vector<std::string> segments;
        std::string segment;
        
        for (char ch : path) {
            if (ch == '/') {
                if (!segment.empty()) {
                    segments.push_back(segment);
                    segment.clear();
                }
            } else {
                segment += ch;
            }
        }
        
        if (!segment.empty()) {
            segments.push_back(segment);
        }
        
        return segments;
    }
};
