#pragma once

#include <string>
#include <chrono>

/**
 * @file ISplunkLogSettings.hpp
 * @brief Interface for Splunk log settings
 * @author Anton Tobolkin
 */

/**
 * @class ISplunkLogSettings
 * @brief Splunk logger configuration interface
 *
 * Provides all settings needed for SplunkLogger.
 * Settings are loaded from environment variables by SplunkLogSettings implementation.
 */
class ISplunkLogSettings {
public:
    virtual ~ISplunkLogSettings() = default;

    /**
     * @brief Get Splunk HTTP Event Collector endpoint URL
     * @return URL for HEC (e.g., http://localhost:8088/services/collector)
     */
    virtual std::string getUrl() const = 0;

    /**
     * @brief Get Splunk HEC token
     * @return Token string for authentication
     */
    virtual std::string getToken() const = 0;

    /**
     * @brief Get Splunk index name
     * @return Index name (default: main)
     */
    virtual std::string getIndex() const = 0;

    /**
     * @brief Get Splunk sourcetype
     * @return Sourcetype string (default: _json)
     */
    virtual std::string getSourceType() const = 0;

    /**
     * @brief Get buffer size before forced flush
     * @return Number of log entries
     */
    virtual size_t getBufferSize() const = 0;

    /**
     * @brief Get flush interval in seconds
     * @return Interval duration
     */
    virtual std::chrono::seconds getFlushInterval() const = 0;
};