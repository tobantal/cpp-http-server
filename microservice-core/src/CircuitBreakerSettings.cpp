#include "application/CircuitBreakerSettings.hpp"

CircuitBreakerSettings::CircuitBreakerSettings(const std::string& prefix)
    : prefix_(prefix)
{
    failureThreshold_ = getEnvInt(prefix + "_CB_FAILURE_THRESHOLD", 5);
    resetTimeoutMs_ = getEnvInt(prefix + "_CB_RESET_TIMEOUT_MS", 30000);
    halfOpenMaxCalls_ = getEnvInt(prefix + "_CB_HALF_OPEN_MAX_CALLS", 3);
}

int CircuitBreakerSettings::getFailureThreshold() const
{
    return failureThreshold_;
}

std::chrono::milliseconds CircuitBreakerSettings::getResetTimeout() const
{
    return std::chrono::milliseconds(resetTimeoutMs_);
}

int CircuitBreakerSettings::getHalfOpenMaxCalls() const
{
    return halfOpenMaxCalls_;
}

int CircuitBreakerSettings::getEnvInt(const std::string& name, int defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stoi(value) : defaultValue;
}

double CircuitBreakerSettings::getEnvDouble(const std::string& name, double defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::stod(value) : defaultValue;
}

bool CircuitBreakerSettings::getEnvBool(const std::string& name, bool defaultValue)
{
    const char* value = std::getenv(name.c_str());
    if (!value) return defaultValue;
    std::string lower;
    for (const char* p = value; *p; ++p)
    {
        lower += static_cast<char>(std::tolower(static_cast<unsigned char>(*p)));
    }
    return lower == "true" || lower == "1" || lower == "yes";
}

std::string CircuitBreakerSettings::getEnvString(const std::string& name, const std::string& defaultValue)
{
    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
}