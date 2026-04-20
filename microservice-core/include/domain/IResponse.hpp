#pragma once
#include <string>
#include <map>
#include <optional>
#include "HttpStatus.hpp"

/**
 * @file IResponse.hpp
 * @brief HTTP response interface
 * @version 2.1
 * @author Anton Tobolkin
 */

/**
 * @struct IResponse
 * @brief Interface for HTTP response
 */
struct IResponse {
    virtual ~IResponse() = default;

    // =========================================================================
    // SETTERS
    // =========================================================================

    /**
     * @brief Set HTTP status code
     * @param code Status code (200, 201, 400, 401, 404, 500, etc.)
     */
    virtual void setStatus(int code) = 0;

    /**
     * @brief Set HTTP status from HttpStatus enum
     * @param status HttpStatus enum value
     */
    virtual void setStatus(HttpStatus status) = 0;

    /**
     * @brief Set response body
     * @param body Response body
     */
    virtual void setBody(const std::string& body) = 0;

    /**
     * @brief Set response header
     * @param name Header name
     * @param value Header value
     */
    virtual void setHeader(const std::string& name, const std::string& value) = 0;

    /**
     * @brief Set Set-Cookie header
     * @param name Cookie name
     * @param value Cookie value
     * @param path Path attribute (default: "/")
     * @param httpOnly HttpOnly flag (default: true)
     * @param secure Secure flag (default: false)
     * @param maxAge Max-Age in seconds (default: -1 = not set)
     */
    virtual void setCookie(const std::string& name,
                            const std::string& value,
                            const std::string& path = "/",
                            bool httpOnly = true,
                            bool secure = false,
                            int maxAge = -1) = 0;

    // =========================================================================
    // GETTERS
    // =========================================================================

    /**
     * @brief Get HTTP status code
     * @return Status code
     */
    virtual int getStatus() const = 0;

    /**
     * @brief Get response body
     * @return Response body
     */
    virtual std::string getBody() const = 0;

    /**
     * @brief Get all response headers
     * @return Map name to value
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;

    /**
     * @brief Get header by name
     * @param name Header name
     * @return Value or nullopt if not set
     */
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;

    // =========================================================================
    // CONVENIENCE METHODS
    // =========================================================================

    /**
     * @brief Set full response result
     * @param code HTTP status code
     * @param contentType Content-Type value
     * @param body Response body
     */
    virtual void setResult(int code,
                           const std::string& contentType,
                           const std::string& body) = 0;

    /**
     * @brief Set full response result with HttpStatus enum
     * @param status HttpStatus enum value
     * @param contentType Content-Type value
     * @param body Response body
     */
    virtual void setResult(HttpStatus status,
                           const std::string& contentType,
                           const std::string& body) = 0;

    // =========================================================================
    // TRACE ID
    // =========================================================================

    /**
     * @brief Set X-Trace-ID in response
     * @param id Trace ID
     */
    virtual void setTraceId(const std::string& id) = 0;
};
