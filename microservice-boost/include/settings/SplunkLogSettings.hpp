#pragma once

#include "settings/ISplunkLogSettings.hpp"
#include <cstdlib>
#include <string>
#include <map>
#include <sstream>

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
 * - <PREFIX>_SPLUNK_HEADERS (semicolon-separated key:value pairs)
 * - <PREFIX>_SPLUNK_BUFFER_SIZE (default: 100)
 * - <PREFIX>_SPLUNK_FLUSH_INTERVAL_SEC (default: 5)
 *
 * Auth header is automatically set to "Splunk {token}".
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
    explicit SplunkLogSettings(const std::string& prefix)
        : prefix_(prefix)
    {
        url_ = getEnvOrDefault((prefix + "_SPLUNK_URL").c_str(),
                               "http://localhost:8088/services/collector");
        token_ = getEnvOrDefault((prefix + "_SPLUNK_TOKEN").c_str(), "");
        index_ = getEnvOrDefault((prefix + "_SPLUNK_INDEX").c_str(), "main");
        sourcetype_ = getEnvOrDefault((prefix + "_SPLUNK_SOURCETYPE").c_str(), "_json");
        headersStr_ = getEnvOrDefault((prefix + "_SPLUNK_HEADERS").c_str(), "");

        bufferSize_ = std::stoul(getEnvOrDefault(
            (prefix + "_SPLUNK_BUFFER_SIZE").c_str(), "100"));
        flushIntervalSec_ = std::stoi(getEnvOrDefault(
            (prefix + "_SPLUNK_FLUSH_INTERVAL_SEC").c_str(), "5"));

        parseHeaders();
    }

    ~SplunkLogSettings() override = default;

    std::string getUrl() const override { return url_; }
    std::string getAuth() const override {
        return token_.empty() ? "" : "Splunk " + token_;
    }
    std::map<std::string, std::string> getHeaders() const override { return headers_; }
    size_t getBufferSize() const override { return bufferSize_; }
    std::chrono::seconds getFlushInterval() const override {
        return std::chrono::seconds(flushIntervalSec_);
    }

    std::string getToken() const override { return token_; }
    std::string getIndex() const override { return index_; }
    std::string getSourceType() const override { return sourcetype_; }

private:
    std::string prefix_;
    std::string url_;
    std::string token_;
    std::string index_;
    std::string sourcetype_;
    std::string headersStr_;
    size_t bufferSize_;
    int flushIntervalSec_;
    std::map<std::string, std::string> headers_;

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : defaultValue;
    }

    void parseHeaders() {
        if (headersStr_.empty()) {
            return;
        }
        std::stringstream ss(headersStr_);
        std::string pair;
        while (std::getline(ss, pair, ';')) {
            auto colonPos = pair.find(':');
            if (colonPos != std::string::npos) {
                std::string key = pair.substr(0, colonPos);
                std::string value = pair.substr(colonPos + 1);
                headers_[key] = value;
            }
        }
    }
};