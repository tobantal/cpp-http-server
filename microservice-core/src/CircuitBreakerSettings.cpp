#include "application/CircuitBreakerSettings.hpp"
#include <algorithm>
#include <cctype>

CircuitBreakerSettings::CircuitBreakerSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix)
    : env_(env), prefix_(prefix)
{
}

std::string CircuitBreakerSettings::toEnvName(const std::string& configKey)
{
    std::string result;
    for (char c : configKey)
    {
        if (c == '.') result += '_';
        else result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string CircuitBreakerSettings::toLower(const std::string& s)
{
    std::string result = s;
    for (auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

int CircuitBreakerSettings::getFailureThreshold() const
{
    return resolve<int>("cb.failureThreshold", 5);
}

std::chrono::milliseconds CircuitBreakerSettings::getResetTimeout() const
{
    return std::chrono::milliseconds(resolve<int>("cb.resetTimeoutMs", 30000));
}

int CircuitBreakerSettings::getHalfOpenMaxCalls() const
{
    return resolve<int>("cb.halfOpenMaxCalls", 3);
}
