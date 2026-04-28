#pragma once

#include "settings/IHttpLogSettings.hpp"
#include <string>
#include <chrono>
#include <map>

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
 * Inherits from IHttpLogSettings for common HTTP logger settings.
 */
class ISplunkLogSettings : public IHttpLogSettings {
public:
    virtual ~ISplunkLogSettings() = default;

    virtual std::string getToken() const = 0;
    virtual std::string getIndex() const = 0;
    virtual std::string getSourceType() const = 0;
};