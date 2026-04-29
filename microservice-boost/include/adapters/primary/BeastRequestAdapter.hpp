#pragma once

#include "domain/IRequest.hpp"
#include "util/IIdGenerator.hpp"
#include "util/UuidGenerator.hpp"
#include "util/PathParamExtractor.hpp"
#include "util/StringUtils.hpp"
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
        std::shared_ptr<IIdGenerator> idGenerator = std::make_shared<UuidGenerator>())
        : req_(req), ip_(clientIp), port_(port), body_(req.body()), idGenerator_(std::move(idGenerator)) {}

    std::string getPath() const override;
    std::vector<std::string> getPathSegments() const override;
    std::string getPathPattern() const override;
    void setPathPattern(const std::string& pattern) override;
    std::optional<std::string> getPathParam(size_t index) const override;
    std::map<std::string, std::string> getQueryParams() const override;
    std::optional<std::string> getQueryParam(const std::string& name) const override;
    void setQueryParam(const std::string& name, const std::string& value) override;
    std::map<std::string, std::string> getParams() const override;
    std::map<std::string, std::string> getHeaders() const override;
    std::optional<std::string> getHeader(const std::string& name) const override;
    void setHeader(const std::string& name, const std::string& value) override;
    void setHeaders(const std::map<std::string, std::string>& headers) override;
    std::string getBody() const override;
    void setBody(const std::string& body) override;
    std::string getMethod() const override;
    std::string getIp() const override;
    int getPort() const override;
    std::optional<std::string> getBearerToken() const override;
    bool isJson() const override;
    std::string getContentType() const override;
    void setAttribute(const std::string& name, const std::string& value) override;
    std::optional<std::string> getAttribute(const std::string& name) const override;
    std::string getTraceId() override;
    void setTraceId(const std::string& id) override;

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
    std::shared_ptr<IIdGenerator> idGenerator_;
};