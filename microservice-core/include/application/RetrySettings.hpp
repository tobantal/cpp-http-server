#pragma once

#include "application/IRetrySettings.hpp"
#include "ports/output/IEnvironment.hpp"
#include <string>
#include <memory>
#include <type_traits>

/**
 * @file RetrySettings.hpp
 * @brief Generic retry settings with 3-tier fallback: ENV - config.json - default
 * @author Anton Tobolkin
 */

/**
 * @class RetrySettings
 * @brief Implements IRetrySettings with ENV → config.json → default fallback
 *
 * resolve() method:
 * 1. Check ENV via std::getenv(envVarName)
 * 2. If not set, check config.json via env_->get<T>(configKey)
 * 3. If not set, use default value
 */
class RetrySettings : public IRetrySettings
{
public:
    RetrySettings(std::shared_ptr<IEnvironment> env, const std::string& prefix);

    int getMaxAttempts() const override;
    std::chrono::milliseconds getBaseDelay() const override;
    double getMultiplier() const override;
    std::chrono::milliseconds getMaxDelay() const override;

private:
    std::shared_ptr<IEnvironment> env_;
    std::string prefix_;

    template<typename T>
    T resolve(const std::string& configKey, T defaultValue) const;

    static std::string toEnvName(const std::string& configKey);
    static std::string toLower(const std::string& s);
};

template<typename T>
T RetrySettings::resolve(const std::string& configKey, T defaultValue) const
{
    std::string fullKey = toLower(prefix_) + "." + configKey;
    std::string envVarName;

    if (configKey == "retry.maxAttempts")
        envVarName = prefix_ + "_RETRY_MAX_ATTEMPTS";
    else if (configKey == "retry.baseDelayMs")
        envVarName = prefix_ + "_RETRY_BASE_DELAY_MS";
    else if (configKey == "retry.multiplier")
        envVarName = prefix_ + "_RETRY_MULTIPLIER";
    else if (configKey == "retry.maxDelayMs")
        envVarName = prefix_ + "_RETRY_MAX_DELAY_MS";
    else
        envVarName = prefix_ + "_" + toEnvName(configKey);

    const char* envValue = std::getenv(envVarName.c_str());
    if (envValue)
    {
        if constexpr (std::is_same_v<T, int>)
        {
            return std::stoi(envValue);
        }
        else if constexpr (std::is_same_v<T, double>)
        {
            return std::stod(envValue);
        }
        else if constexpr (std::is_same_v<T, std::string>)
        {
            return std::string(envValue);
        }
        else
        {
            return T();
        }
    }

    if (env_)
    {
        try
        {
            return env_->get<T>(fullKey);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    return defaultValue;
}
