#pragma once

#include "settings/ISplunkLogSettings.hpp"
#include <cstdlib>
#include <string>

/**
 * @file SplunkLogSettings.hpp
 * @brief Splunk logger settings loaded from environment variables
 * @author Anton Tobolkin
 */

/**
 * @class SplunkLogSettings
 * @brief Splunk HTTP Event Collector settings
 *
 * Reads settings from environment variables with the given prefix:
 * - <PREFIX>_SPLUNK_URL (default: http://localhost:8088/services/collector)
 * - <PREFIX>_SPLUNK_TOKEN
 * - <PREFIX>_SPLUNK_INDEX (default: main)
 * - <PREFIX>_SPLUNK_SOURCETYPE (default: _json)
 * - <PREFIX>_SPLUNK_BUFFER_SIZE (default: 100)
 * - <PREFIX>_SPLUNK_FLUSH_INTERVAL_SEC (default: 5)
 *
 * @example
 *   SplunkLogSettings settings("APP");
 *   // reads APP_SPLUNK_URL, APP_SPLUNK_TOKEN, etc.
 */
class SplunkLogSettings : public ISplunkLogSettings {
public:
    /**
     * @brief Construct SplunkLogSettings with prefix
     * @param prefix ENV prefix (e.g., "APP", "SERVICE")
     */
    explicit SplunkLogSettings(const std::string& prefix);

    ~SplunkLogSettings() override = default;

    std::string getUrl() const override;
    std::string getToken() const override;
    std::string getIndex() const override;
    std::string getSourceType() const override;
    size_t getBufferSize() const override;
    std::chrono::seconds getFlushInterval() const override;

private:
    std::string prefix_;
    std::string url_;
    std::string token_;
    std::string index_;
    std::string sourcetype_;
    size_t bufferSize_;
    int flushIntervalSec_;

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue);
};