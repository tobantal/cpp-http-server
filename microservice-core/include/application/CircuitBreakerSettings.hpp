#pragma once

#include "application/ICircuitBreakerSettings.hpp"
#include <string>

/**
 * @file CircuitBreakerSettings.hpp
 * @brief Circuit breaker settings from environment variables
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreakerSettings
 * @brief Circuit breaker configuration from environment
 *
 * @par Environment Variables
 * - <PREFIX>_CB_FAILURE_THRESHOLD (default: 5)
 * - <PREFIX>_CB_RESET_TIMEOUT_MS (default: 30000)
 * - <PREFIX>_CB_HALF_OPEN_MAX_CALLS (default: 3)
 */
class CircuitBreakerSettings : public ICircuitBreakerSettings
{
public:
    /**
     * @brief Construct CircuitBreakerSettings
     * @param prefix Environment variable prefix (e.g., "HTTP")
     */
    explicit CircuitBreakerSettings(const std::string& prefix);

    int getFailureThreshold() const override;
    std::chrono::milliseconds getResetTimeout() const override;
    int getHalfOpenMaxCalls() const override;

private:
    static int getEnvInt(const std::string& name, int defaultValue);
    static double getEnvDouble(const std::string& name, double defaultValue);
    static bool getEnvBool(const std::string& name, bool defaultValue);
    static std::string getEnvString(const std::string& name, const std::string& defaultValue);

    std::string prefix_;
    int failureThreshold_;
    int resetTimeoutMs_;
    int halfOpenMaxCalls_;
};