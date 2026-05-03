#include "application/RetrySettings.hpp"

RetrySettings::RetrySettings(const std::string& prefix)
    : prefix_(prefix)
{
    maxAttempts_ = getEnvInt(prefix + "_RETRY_MAX_ATTEMPTS", 3);
    baseDelayMs_ = getEnvInt(prefix + "_RETRY_BASE_DELAY_MS", 1000);
    multiplier_ = getEnvDouble(prefix + "_RETRY_MULTIPLIER", 2.0);
    maxDelayMs_ = getEnvInt(prefix + "_RETRY_MAX_DELAY_MS", 30000);
}

int RetrySettings::getMaxAttempts() const
{
    return maxAttempts_;
}

std::chrono::milliseconds RetrySettings::getBaseDelay() const
{
    return std::chrono::milliseconds(baseDelayMs_);
}

double RetrySettings::getMultiplier() const
{
    return multiplier_;
}

std::chrono::milliseconds RetrySettings::getMaxDelay() const
{
    return std::chrono::milliseconds(maxDelayMs_);
}

int RetrySettings::getEnvInt(const std::string& name, int defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stoi(value) : defaultValue;
}

double RetrySettings::getEnvDouble(const std::string& name, double defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stod(value) : defaultValue;
}