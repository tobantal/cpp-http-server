#pragma once

#include "application/ICircuitBreakerSettings.hpp"
#include "ports/output/IEnvironment.hpp"
#include <string>
#include <memory>
#include <type_traits>
#include <cctype>
#include <algorithm>

/**
 * @file CircuitBreakerSettings.hpp
 * @brief Circuit breaker settings with 3-tier fallback: ENV - config.json - default
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreakerSettings
 * @brief Circuit breaker configuration with ENV → config.json → default fallback
 *
 * resolve() method:
 * 1. Check ENV via std::getenv(envVarName)
 * 2. If not set, check config.json via env_->get<T>(configKey)
 * 3. If not set, use default value
 */
class CircuitBreakerSettings : public ICircuitBreakerSettings
{
public:
    CircuitBreakerSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix);

    int getFailureThreshold() const override;
    std::chrono::milliseconds getResetTimeout() const override;
    int getHalfOpenMaxCalls() const override;

private:
    std::shared_ptr<IEnvironment> env_;
    std::string prefix_;

    template<typename T>
    T resolve(const std::string& configKey, T defaultValue) const;

    static std::string toEnvName(const std::string& configKey);
    static std::string toLower(const std::string& s);
};

template<typename T>
T CircuitBreakerSettings::resolve(const std::string& configKey, T defaultValue) const
{
    std::string fullKey = toLower(prefix_) + "." + configKey;
    std::string envVarName;

    if (configKey == "cb.failureThreshold")
        envVarName = prefix_ + "_CB_FAILURE_THRESHOLD";
    else if (configKey == "cb.resetTimeoutMs")
        envVarName = prefix_ + "_CB_RESET_TIMEOUT_MS";
    else if (configKey == "cb.halfOpenMaxCalls")
        envVarName = prefix_ + "_CB_HALF_OPEN_MAX_CALLS";
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
        else if constexpr (std::is_same_v<T, bool>)
        {
            std::string lower;
            for (const char* p = envValue; *p; ++p)
                lower += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
            return lower == "true" || lower == "1" || lower == "yes";
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
