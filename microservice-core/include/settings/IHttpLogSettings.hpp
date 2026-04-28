#pragma once

#include <string>
#include <chrono>
#include <map>

/**
 * @file IHttpLogSettings.hpp
 * @brief Interface for HTTP log settings
 * @author Anton Tobolkin
 */

/**
 * @class IHttpLogSettings
 * @brief Interface for HTTP logger configuration
 *
 * Provides all settings needed for HttpLogger:
 * - HTTP endpoint URL
 * - Authentication
 * - Buffering parameters
 * - Fallback behavior
 */
class IHttpLogSettings {
public:
    virtual ~IHttpLogSettings() = default;

    /**
     * @brief Get HTTP endpoint URL
     * @return URL for HTTP POST requests
     */
    virtual std::string getUrl() const = 0;

    /**
     * @brief Get HTTP authentication header value
     * @return Auth string (e.g., "Bearer xyz" or "Splunk xyz")
     */
    virtual std::string getAuth() const = 0;

    /**
     * @brief Get additional HTTP headers
     * @return Map of header name to value
     */
    virtual std::map<std::string, std::string> getHeaders() const = 0;

    /**
     * @brief Get maximum buffer size before forced flush
     * @return Number of log entries
     */
    virtual size_t getBufferSize() const = 0;

    /**
     * @brief Get flush interval in seconds
     * @return Interval duration
     */
    virtual std::chrono::seconds getFlushInterval() const = 0;
};