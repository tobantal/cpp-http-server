#pragma once

#include "domain/IRequest.hpp"
#include "util/IIdGenerator.hpp"
#include "util/UuidGenerator.hpp"
#include "util/StringUtils.hpp"
#include "util/PathParamExtractor.hpp"
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <cctype>

/**
 * @file SimpleRequest.hpp
 * @brief Simple IRequest implementation for testing
 * @author Anton Tobolkin
 */

/**
 * @struct SimpleRequest
 * @brief Simple IRequest implementation
 */
struct SimpleRequest : IRequest
{
    SimpleRequest(const std::string& method,
                  const std::string& path,
                  const std::string& body,
                  const std::string& ip,
                  int port,
                  const std::map<std::string, std::string>& headers = {},
                  std::shared_ptr<IIdGenerator> idGenerator = std::make_shared<UuidGenerator>())
        : method_(method), path_(path), body_(body), ip_(ip), port_(port), headers_(headers), idGenerator_(std::move(idGenerator))
    {
    }

    SimpleRequest()
        : method_("GET"), path_("/"), body_(""), ip_("127.0.0.1"), port_(80), idGenerator_(std::make_shared<UuidGenerator>())
    {
    }

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
    void setObject(const std::string& name, std::shared_ptr<IEnvironment> obj) override;
    std::optional<std::shared_ptr<IEnvironment>> getObject(const std::string& name) const override;
    std::string getTraceId() override;
    void setTraceId(const std::string& id) override;
    void setMethod(const std::string& method);
    void setPath(const std::string& path);
    void setIp(const std::string& ip);
    void setPort(int port);

private:
    /** @brief HTTP method (GET, POST, etc.) */
    std::string method_;

    /** @brief Request path */
    std::string path_;

    /** @brief Request body */
    std::string body_;

    /** @brief Client IP address */
    std::string ip_;

    /** @brief Client port */
    int port_;

    /** @brief Registered path pattern for param extraction */
    std::string pathPattern_;

    /** @brief HTTP headers */
    std::map<std::string, std::string> headers_;

    /** @brief Query parameters */
    std::map<std::string, std::string> queryParams_;

    /** @brief Request attributes (custom key-value) */
    std::map<std::string, std::string> attributes_;

    /** @brief Request objects (shared ptrs to IEnvironment) */
    std::map<std::string, std::shared_ptr<IEnvironment>> objects_;

    /** @brief Trace ID generator */
    std::shared_ptr<IIdGenerator> idGenerator_;
};