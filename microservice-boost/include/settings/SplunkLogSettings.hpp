#pragma once

#include "settings/ISplunkLogSettings.hpp"
#include "ports/output/IEnvironment.hpp"
#include <string>
#include <memory>

/**
 * @file SplunkLogSettings.hpp
 * @brief Splunk logger settings with 3-tier fallback: ENV - config.json - default
 * @author Anton Tobolkin
 */

/**
 * @class SplunkLogSettings
 * @brief Splunk HTTP Event Collector settings with ENV → config.json → default fallback
 *
 * Naming: config key "splunk.url" → ENV var "SPLUNK_URL" (uppercase, dots → underscores)
 *
 * resolve() method:
 * 1. Check ENV via std::getenv(envVarName)
 * 2. If not set, check config.json via env_->get<T>(configKey)
 * 3. If not set, use default value
 *
 * @example
 *   SplunkLogSettings settings(env, "APP");
 *   // reads APP_SPLUNK_URL, APP_SPLUNK_TOKEN, etc.
 *   // config keys: splunk.url, splunk.token, splunk.index...
 */
class SplunkLogSettings : public ISplunkLogSettings {
public:
    SplunkLogSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix);

    explicit SplunkLogSettings(const std::string& prefix);

    ~SplunkLogSettings() override = default;

    std::string getUrl() const override;
    std::string getToken() const override;
    std::string getIndex() const override;
    std::string getSourceType() const override;
    size_t getBufferSize() const override;
    std::chrono::seconds getFlushInterval() const override;

private:
    std::shared_ptr<IEnvironment> env_;
    std::string prefix_;

    template<typename T>
    T resolve(const std::string& configKey, T defaultValue) const;

    static std::string toEnvName(const std::string& configKey);
};
