#pragma once

#include "application/IHttpRetrySettings.hpp"
#include <string>

/**
 * @file HttpRetrySettings.hpp
 * @brief HTTP-specific retry settings implementation
 * @author Anton Tobolkin
 */

/**
 * @class HttpRetrySettings
 * @brief HTTP retry settings from environment variables
 *
 * Extends IHttpRetrySettings with HTTP-specific configuration:
 * retryable status codes and network error retry flag.
 *
 * @par Environment Variables
 * - <PREFIX>_RETRY_MAX_ATTEMPTS (default: 3)
 * - <PREFIX>_RETRY_BASE_DELAY_MS (default: 1000)
 * - <PREFIX>_RETRY_MULTIPLIER (default: 2.0)
 * - <PREFIX>_RETRY_MAX_DELAY_MS (default: 30000)
 * - <PREFIX>_RETRY_STATUSES (default: "500,502,503,504")
 * - <PREFIX>_RETRY_ON_NETWORK_ERROR (default: true)
 */
class HttpRetrySettings : public IHttpRetrySettings
{
public:
    /**
     * @brief Construct HttpRetrySettings
     * @param prefix Environment variable prefix (e.g., "HTTP")
     */
    explicit HttpRetrySettings(const std::string& prefix);

    int getMaxAttempts() const override;
    std::chrono::milliseconds getBaseDelay() const override;
    double getMultiplier() const override;
    std::chrono::milliseconds getMaxDelay() const override;
    const std::set<int>& getRetryableStatuses() const override;
    bool isRetryOnNetworkErrorEnabled() const override;

private:
    void parseStatuses(const std::string& statusStr);

    static int getEnvInt(const std::string& name, int defaultValue);
    static double getEnvDouble(const std::string& name, double defaultValue);
    static bool getEnvBool(const std::string& name, bool defaultValue);
    static std::string getEnvString(const std::string& name, const std::string& defaultValue);

    std::string prefix_;
    int maxAttempts_;
    int baseDelayMs_;
    double multiplier_;
    int maxDelayMs_;
    bool retryOnNetworkError_;
    std::set<int> retryableStatuses_;
};