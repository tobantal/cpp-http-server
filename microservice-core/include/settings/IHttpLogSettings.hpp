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
 */
class IHttpLogSettings {
public:
    virtual ~IHttpLogSettings() = default;

    virtual std::string getUrl() const = 0;
    virtual std::string getAuth() const = 0;
    virtual std::map<std::string, std::string> getHeaders() const = 0;
    virtual size_t getBufferSize() const = 0;
    virtual std::chrono::seconds getFlushInterval() const = 0;
};