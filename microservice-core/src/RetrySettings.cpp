#include "application/RetrySettings.hpp"
#include <algorithm>
#include <cctype>

RetrySettings::RetrySettings(std::shared_ptr<IEnvironment> env, const std::string& prefix)
    : env_(env), prefix_(prefix)
{
}

std::string RetrySettings::toEnvName(const std::string& configKey)
{
    std::string result;
    for (char c : configKey)
    {
        if (c == '.') result += '_';
        else result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string RetrySettings::toLower(const std::string& s)
{
    std::string result = s;
    for (auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

int RetrySettings::getMaxAttempts() const
{
    return resolve<int>("retry.maxAttempts", 3);
}

std::chrono::milliseconds RetrySettings::getBaseDelay() const
{
    return std::chrono::milliseconds(resolve<int>("retry.baseDelayMs", 1000));
}

double RetrySettings::getMultiplier() const
{
    return resolve<double>("retry.multiplier", 2.0);
}

std::chrono::milliseconds RetrySettings::getMaxDelay() const
{
    return std::chrono::milliseconds(resolve<int>("retry.maxDelayMs", 30000));
}
