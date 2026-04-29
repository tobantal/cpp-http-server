#pragma once

#include "application/IRetrySettings.hpp"
#include <cstdlib>
#include <string>

/**
 * @file RetrySettings.hpp
 * @brief Generic retry settings from environment variables
 * @author Anton Tobolkin
 */

/**
 * @class RetrySettings
 * @brief Implements IRetrySettings reading from environment variables
 *
 * Reads configuration from ENV with configurable prefix.
 *
 * @par Environment Variables
 * - <PREFIX>_RETRY_MAX_ATTEMPTS (default: 3)
 * - <PREFIX>_RETRY_BASE_DELAY_MS (default: 1000)
 * - <PREFIX>_RETRY_MULTIPLIER (default: 2.0)
 * - <PREFIX>_RETRY_MAX_DELAY_MS (default: 30000)
 */
class RetrySettings : public IRetrySettings
{
public:
    /**
     * @brief Construct RetrySettings
     * @param prefix Environment variable prefix (e.g., "HTTP", "BROKER")
     */
    explicit RetrySettings(const std::string& prefix)
        : prefix_(prefix)
    {
        maxAttempts_ = getEnvInt(prefix + "_RETRY_MAX_ATTEMPTS", 3);
        baseDelayMs_ = getEnvInt(prefix + "_RETRY_BASE_DELAY_MS", 1000);
        multiplier_ = getEnvDouble(prefix + "_RETRY_MULTIPLIER", 2.0);
        maxDelayMs_ = getEnvInt(prefix + "_RETRY_MAX_DELAY_MS", 30000);
    }

    int getMaxAttempts() const override { return maxAttempts_; }
    std::chrono::milliseconds getBaseDelay() const override { return std::chrono::milliseconds(baseDelayMs_); }
    double getMultiplier() const override { return multiplier_; }
    std::chrono::milliseconds getMaxDelay() const override { return std::chrono::milliseconds(maxDelayMs_); }

private:
    std::string prefix_;
    int maxAttempts_;
    int baseDelayMs_;
    double multiplier_;
    int maxDelayMs_;

    static int getEnvInt(const std::string& name, int defaultValue) {
        const char* value = std::getenv(name.c_str());
        return value ? std::stoi(value) : defaultValue;
    }

    static double getEnvDouble(const std::string& name, double defaultValue) {
        const char* value = std::getenv(name.c_str());
        return value ? std::stod(value) : defaultValue;
    }
};