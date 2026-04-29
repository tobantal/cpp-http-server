#pragma once

#include "application/IHttpRetrySettings.hpp"
#include <cstdlib>
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
    explicit HttpRetrySettings(const std::string& prefix)
        : prefix_(prefix)
    {
        maxAttempts_ = getEnvInt(prefix + "_RETRY_MAX_ATTEMPTS", 3);
        baseDelayMs_ = getEnvInt(prefix + "_RETRY_BASE_DELAY_MS", 1000);
        multiplier_ = getEnvDouble(prefix + "_RETRY_MULTIPLIER", 2.0);
        maxDelayMs_ = getEnvInt(prefix + "_RETRY_MAX_DELAY_MS", 30000);
        retryOnNetworkError_ = getEnvBool(prefix + "_RETRY_ON_NETWORK_ERROR", true);
        parseStatuses(getEnvString(prefix + "_RETRY_STATUSES", "500,502,503,504"));
    }

    int getMaxAttempts() const override { return maxAttempts_; }
    std::chrono::milliseconds getBaseDelay() const override { return std::chrono::milliseconds(baseDelayMs_); }
    double getMultiplier() const override { return multiplier_; }
    std::chrono::milliseconds getMaxDelay() const override { return std::chrono::milliseconds(maxDelayMs_); }
    const std::set<int>& getRetryableStatuses() const override { return retryableStatuses_; }
    bool isRetryOnNetworkErrorEnabled() const override { return retryOnNetworkError_; }

private:
    std::string prefix_;
    int maxAttempts_;
    int baseDelayMs_;
    double multiplier_;
    int maxDelayMs_;
    bool retryOnNetworkError_;
    std::set<int> retryableStatuses_;

    static int getEnvInt(const std::string& name, int defaultValue) {
        const char* value = std::getenv(name.c_str());
        return value ? std::stoi(value) : defaultValue;
    }

    static double getEnvDouble(const std::string& name, double defaultValue) {
        const char* value = std::getenv(name.c_str());
        return value ? std::stod(value) : defaultValue;
    }

    static bool getEnvBool(const std::string& name, bool defaultValue) {
        const char* value = std::getenv(name.c_str());
        if (!value) return defaultValue;
        std::string lower;
        for (const char* p = value; *p; ++p) {
            lower += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
        }
        return lower == "true" || lower == "1" || lower == "yes";
    }

    static std::string getEnvString(const std::string& name, const std::string& defaultValue) {
        const char* value = std::getenv(name.c_str());
        return value ? std::string(value) : defaultValue;
    }

    void parseStatuses(const std::string& statusStr) {
        std::stringstream ss(statusStr);
        std::string token;
        while (std::getline(ss, token, ',')) {
            retryableStatuses_.insert(std::stoi(token));
        }
    }
};