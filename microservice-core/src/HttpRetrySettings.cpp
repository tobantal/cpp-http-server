#include "application/HttpRetrySettings.hpp"
#include <sstream>
#include <algorithm>
#include <cctype>

HttpRetrySettings::HttpRetrySettings(std::shared_ptr<IEnvironment> env, const std::string& prefix)
    : env_(env), prefix_(prefix), retryOnNetworkError_(true)
{
    parseStatuses(resolve<std::string>("retry.statuses", "500,502,503,504"));
}

std::string HttpRetrySettings::toEnvName(const std::string& configKey)
{
    std::string result;
    for (char c : configKey)
    {
        if (c == '.') result += '_';
        else result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string HttpRetrySettings::toLower(const std::string& s)
{
    std::string result = s;
    for (auto& c : result) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return result;
}

void HttpRetrySettings::parseStatuses(const std::string& statusStr)
{
    std::stringstream ss(statusStr);
    std::string token;
    while (std::getline(ss, token, ',')) {
        retryableStatuses_.insert(std::stoi(token));
    }
}

int HttpRetrySettings::getMaxAttempts() const
{
    return resolve<int>("retry.maxAttempts", 3);
}

std::chrono::milliseconds HttpRetrySettings::getBaseDelay() const
{
    return std::chrono::milliseconds(resolve<int>("retry.baseDelayMs", 1000));
}

double HttpRetrySettings::getMultiplier() const
{
    return resolve<double>("retry.multiplier", 2.0);
}

std::chrono::milliseconds HttpRetrySettings::getMaxDelay() const
{
    return std::chrono::milliseconds(resolve<int>("retry.maxDelayMs", 30000));
}

const std::set<int>& HttpRetrySettings::getRetryableStatuses() const
{
    return retryableStatuses_;
}

bool HttpRetrySettings::isRetryOnNetworkErrorEnabled() const
{
    return resolve<bool>("retry.onNetworkError", true);
}
