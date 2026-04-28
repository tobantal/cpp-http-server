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
 * Provides all settings needed for SplunkLogger:
 * - HTTP endpoint URL
 * - Splunk token authentication (Authorization: Splunk {token})
 * - Index and sourcetype
 * - Buffering parameters
 */
class ISplunkLogSettings {
public:
    virtual ~ISplunkLogSettings() = default;

    virtual std::string getUrl() const = 0;
    virtual std::string getToken() const = 0;
    virtual std::string getIndex() const = 0;
    virtual std::string getSourceType() const = 0;
    virtual size_t getBufferSize() const = 0;
    virtual std::chrono::seconds getFlushInterval() const = 0;
};