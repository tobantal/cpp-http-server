#pragma once

#include "settings/IHttpLogSettings.hpp"
#include <cstdlib>
#include <string>
#include <map>
#include <sstream>

/**
 * @file HttpLogSettings.hpp
 * @brief HTTP log settings loaded from environment variables
 * @author Anton Tobolkin
 */

/**
 * @class HttpLogSettings
 * @brief HTTP logger settings with configurable ENV prefix
 *
 * Reads settings from environment variables with the given prefix:
 * - <PREFIX>_LOG_URL
 * - <PREFIX>_LOG_AUTH
 * - <PREFIX>_LOG_HEADERS (semicolon-separated key:value pairs)
 * - <PREFIX>_LOG_BUFFER_SIZE (default: 100)
 * - <PREFIX>_LOG_FLUSH_INTERVAL_SEC (default: 5)
 *
 * @example
 *   HttpLogSettings settings("HTTP");
 *   // reads HTTP_LOG_URL, HTTP_LOG_AUTH, etc.
 */
class HttpLogSettings : public IHttpLogSettings {
public:
    /**
     * @brief Construct HttpLogSettings with prefix
     * @param prefix ENV prefix (e.g., "HTTP", "APP")
     * @param defaultUrl Default URL if not set (default: empty)
     */
    explicit HttpLogSettings(const std::string& prefix,
                              const std::string& defaultUrl = "")
        : prefix_(prefix)
        , defaultUrl_(defaultUrl)
    {
        url_ = getEnvOrDefault((prefix + "_LOG_URL").c_str(), defaultUrl);
        auth_ = getEnvOrDefault((prefix + "_LOG_AUTH").c_str(), "");
        headersStr_ = getEnvOrDefault((prefix + "_LOG_HEADERS").c_str(), "");

        bufferSize_ = std::stoul(getEnvOrDefault(
            (prefix + "_LOG_BUFFER_SIZE").c_str(), "100"));
        flushIntervalSec_ = std::stoi(getEnvOrDefault(
            (prefix + "_LOG_FLUSH_INTERVAL_SEC").c_str(), "5"));

        parseHeaders();
    }

    ~HttpLogSettings() override = default;

    std::string getUrl() const override { return url_; }
    std::string getAuth() const override { return auth_; }
    std::map<std::string, std::string> getHeaders() const override { return headers_; }
    size_t getBufferSize() const override { return bufferSize_; }
    std::chrono::seconds getFlushInterval() const override {
        return std::chrono::seconds(flushIntervalSec_);
    }

private:
    std::string prefix_;
    std::string defaultUrl_;
    std::string url_;
    std::string auth_;
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