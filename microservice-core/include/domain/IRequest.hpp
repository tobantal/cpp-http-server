#pragma once
#include <string>
#include <map>
#include <vector>
#include <optional>

/**
 * @file IRequest.hpp
 * @brief HTTP request interface
 * @version 2.0
 * @author Anton Tobolkin
 */

/**
 * @struct IRequest
 * @brief Interface for HTTP request
 */
struct IRequest {
    virtual ~IRequest() = default;

    // =========================================================================
    // PATH
    // =========================================================================

    /**
     * @brief Get request path without query string, URL-decoded
     *
     * Contract:
     * - Path component only (everything before '?' is stripped)
     * - URL-decoded (%20 → space, %C3%BC → ü, + → space)
     * - No trailing slash normalization (preserved as-is)
     * - Leading '/' is always present
     *
     * Examples:
     *   Request target              → getPath() result
     *   /api/v1/orders               → /api/v1/orders
     *   /api/v1/my%20order?id=5     → /api/v1/my order
     *   /api/v1/orders?sort=asc     → /api/v1/orders
     *
     * @return URL-decoded path without query string
     */
    virtual std::string getPath() const = 0;

    /**
     * @brief Get path segments
     * @return Vector of segments (without empty elements)
     */
    virtual std::vector<std::string> getPathSegments() const = 0;

    // =========================================================================
    // PATH PARAMETERS
    // =========================================================================

    /**
     * @brief Get the pattern that matched the handler
     * @return Pattern or empty string if not set
     */
    virtual std::string getPathPattern() const = 0;

    /**
     * @brief Set the route pattern
     * @param pattern Pattern with wildcards
     */
    virtual void setPathPattern(const std::string& pattern) = 0;

    /**
     * @brief Get path parameter by wildcard index
     * @param index Wildcard index in pattern (starting at 0)
     * @return Parameter value or nullopt if index out of range
     */
    virtual std::optional<std::string> getPathParam(size_t index) const = 0;

    // =========================================================================
    // QUERY PARAMETERS
    // =========================================================================

    /**
     * @brief Get all query parameters
     * @return Map name to value
     */
    virtual std::map<std::string, std::string> getQueryParams() const = 0;

    /**
     * @brief Get query parameter by name
     * @param name Parameter name
     * @return Value or nullopt if not found
     */
    virtual std::optional<std::string> getQueryParam(const std::string& name) const = 0;

    /**
     * @brief Set query parameter
     * @param name Parameter name
     * @param value Parameter value
     */
    virtual void setQueryParam(const std::string& name, const std::string& value) = 0;

    /**
     * @deprecated Use getQueryParams()
     * @brief Alias for backward compatibility
     */
    virtual std::map<std::string, std::string> getParams() const {
        return getQueryParams();
    }

    // =========================================================================
    // HEADERS
    // =========================================================================

    /**
     * @brief Get all HTTP headers
     * @return Map name to value
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;

    /**
     * @brief Get header value by name
     * @param name Header name
     * @return Value or nullopt if not found
     */
    virtual std::optional<std::string> getHeader(const std::string& name) const = 0;

    /**
     * @brief Set header
     * @param name Header name
     * @param value Header value
     */
    virtual void setHeader(const std::string& name, const std::string& value) = 0;

    /**
     * @brief Set multiple headers
     * @param headers Map name to value
     */
    virtual void setHeaders(const std::map<std::string, std::string>& headers) = 0;

    // =========================================================================
    // BODY
    // =========================================================================

    /**
     * @brief Get request body
     * @return Request body as string
     */
    virtual std::string getBody() const = 0;

    /**
     * @brief Set request body
     * @param body Request body
     */
    virtual void setBody(const std::string& body) = 0;

    // =========================================================================
    // METHOD
    // =========================================================================

    /**
     * @brief Get HTTP method
     * @return Method in uppercase (GET, POST, PUT, DELETE, PATCH, etc.)
     */
    virtual std::string getMethod() const = 0;

    // =========================================================================
    // CONNECTION INFO
    // =========================================================================

    /**
     * @brief Get IP address
     * @return Client IP (for incoming) or target IP (for outgoing)
     */
    virtual std::string getIp() const = 0;

    /**
     * @brief Get port
     * @return Port (default 80 for incoming, target for outgoing)
     */
    virtual int getPort() const = 0;

    // =========================================================================
    // CONVENIENCE METHODS
    // =========================================================================

    /**
     * @brief Extract Bearer token from Authorization header
     * @return Token without "Bearer " prefix or nullopt
     */
    virtual std::optional<std::string> getBearerToken() const = 0;

    /**
     * @brief Check if Content-Type is JSON
     * @return true if Content-Type contains "json"
     */
    virtual bool isJson() const = 0;

    /**
     * @brief Get Content-Type
     * @return Content-Type value or empty string if not set
     */
    virtual std::string getContentType() const = 0;

    // =========================================================================
    // ATTRIBUTES
    // =========================================================================

    /**
     * @brief Set request attribute
     * @param name Attribute name
     * @param value Attribute value
     */
    virtual void setAttribute(const std::string& name, const std::string& value) = 0;

    /**
     * @brief Get request attribute
     * @param name Attribute name
     * @return Value or nullopt if not set
     */
    virtual std::optional<std::string> getAttribute(const std::string& name) const = 0;

    // =========================================================================
    // TRACE ID
    // =========================================================================

    /**
     * @brief Get request trace ID
     * @return Existing X-Trace-ID from header or generated UUID v4
     */
    virtual std::string getTraceId() = 0;

    /**
     * @brief Set request trace ID
     * @param id Trace ID
     */
    virtual void setTraceId(const std::string& id) = 0;
};
