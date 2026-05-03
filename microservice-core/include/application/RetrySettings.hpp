#pragma once

#include "application/IRetrySettings.hpp"
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
    explicit RetrySettings(const std::string& prefix);

    int getMaxAttempts() const override;
    std::chrono::milliseconds getBaseDelay() const override;
    double getMultiplier() const override;
    std::chrono::milliseconds getMaxDelay() const override;

private:
    static int getEnvInt(const std::string& name, int defaultValue);
    static double getEnvDouble(const std::string& name, double defaultValue);

    std::string prefix_;
    int maxAttempts_;
    int baseDelayMs_;
    double multiplier_;
    int maxDelayMs_;
};