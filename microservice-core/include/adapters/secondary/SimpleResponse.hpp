#pragma once

#include "domain/IResponse.hpp"
#include "util/StringUtils.hpp"
#include <string>
#include <map>
#include <optional>

/**
 * @file SimpleResponse.hpp
 * @brief Simple IResponse implementation for testing
 * @author Anton Tobolkin
 */

/**
 * @class SimpleResponse
 * @brief Simple IResponse implementation
 */
class SimpleResponse : public IResponse
{
public:
    /**
     * @brief Construct SimpleResponse with initial status and body
     * @param status HTTP status code
     * @param body Response body
     */
    SimpleResponse(int status = 200, const std::string& body = "");

    void setStatus(int code) override;
    void setStatus(HttpStatus status) override;
    void setBody(const std::string& body) override;
    void setHeader(const std::string& name, const std::string& value) override;
    void setCookie(const std::string& name,
                    const std::string& value,
                    const std::string& path = "/",
                    bool httpOnly = true,
                    bool secure = false,
                    int maxAge = -1) override;
    int getStatus() const override;
    std::string getBody() const override;
    std::map<std::string, std::string> getHeaders() const override;
    std::optional<std::string> getHeader(const std::string& name) const override;
    void setResult(int code,
                   const std::string& contentType,
                   const std::string& body) override;
    void setResult(HttpStatus status,
                   const std::string& contentType,
                   const std::string& body) override;
    void setTraceId(const std::string& id) override;

private:
    int status_;
    std::string body_;
    std::map<std::string, std::string> headers_;
};