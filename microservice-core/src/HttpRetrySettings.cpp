#include "application/HttpRetrySettings.hpp"
#include <sstream>

HttpRetrySettings::HttpRetrySettings(const std::string& prefix)
    : prefix_(prefix)
{
    maxAttempts_ = getEnvInt(prefix + "_RETRY_MAX_ATTEMPTS", 3);
    baseDelayMs_ = getEnvInt(prefix + "_RETRY_BASE_DELAY_MS", 1000);
    multiplier_ = getEnvDouble(prefix + "_RETRY_MULTIPLIER", 2.0);
    maxDelayMs_ = getEnvInt(prefix + "_RETRY_MAX_DELAY_MS", 30000);
    retryOnNetworkError_ = getEnvBool(prefix + "_RETRY_ON_NETWORK_ERROR", true);
    parseStatuses(getEnvString(prefix + "_RETRY_STATUSES", "500,502,503,504"));
}

int HttpRetrySettings::getMaxAttempts() const
{
    return maxAttempts_;
}

std::chrono::milliseconds HttpRetrySettings::getBaseDelay() const
{
    return std::chrono::milliseconds(baseDelayMs_);
}

double HttpRetrySettings::getMultiplier() const
{
    return multiplier_;
}

std::chrono::milliseconds HttpRetrySettings::getMaxDelay() const
{
    return std::chrono::milliseconds(maxDelayMs_);
}

const std::set<int>& HttpRetrySettings::getRetryableStatuses() const
{
    return retryableStatuses_;
}

bool HttpRetrySettings::isRetryOnNetworkErrorEnabled() const
{
    return retryOnNetworkError_;
}

void HttpRetrySettings::parseStatuses(const std::string& statusStr)
{
    std::stringstream ss(statusStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        retryableStatuses_.insert(std::stoi(token));
    }
}

int HttpRetrySettings::getEnvInt(const std::string& name, int defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stoi(value) : defaultValue;
}

double HttpRetrySettings::getEnvDouble(const std::string& name, double defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stod(value) : defaultValue;
}

bool HttpRetrySettings::getEnvBool(const std::string& name, bool defaultValue)
{
    const char* value = std::getenv(name.c_str());
    if (!value) return defaultValue;
    std::string lower;
    for (const char* p = value; *p; ++p) {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
    return lower == "true" || lower == "1" || lower == "yes";
}

std::string HttpRetrySettings::getEnvString(const std::string& name, const std::string& defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
}