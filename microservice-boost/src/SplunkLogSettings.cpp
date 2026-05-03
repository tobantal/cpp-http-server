#include "settings/SplunkLogSettings.hpp"
#include <cstdlib>
#include <string>
#include <type_traits>
#include <algorithm>
#include <cctype>

SplunkLogSettings::SplunkLogSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix)
    : env_(env), prefix_(prefix)
{
}

SplunkLogSettings::SplunkLogSettings(const std::string& prefix)
    : env_(nullptr), prefix_(prefix)
{
}

std::string SplunkLogSettings::toEnvName(const std::string& configKey)
{
    std::string result;
    for (char c : configKey)
    {
        if (c == '.')
        {
            result += '_';
        }
        else
        {
            result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
    }
    return result;
}

std::string SplunkLogSettings::getUrl() const
{
    return resolve<std::string>("splunk.url", "http://localhost:8088/services/collector");
}

std::string SplunkLogSettings::getToken() const
{
    return resolve<std::string>("splunk.token", "");
}

std::string SplunkLogSettings::getIndex() const
{
    return resolve<std::string>("splunk.index", "main");
}

std::string SplunkLogSettings::getSourceType() const
{
    return resolve<std::string>("splunk.sourcetype", "_json");
}

size_t SplunkLogSettings::getBufferSize() const
{
    return resolve<size_t>("splunk.buffer_size", 100);
}

std::chrono::seconds SplunkLogSettings::getFlushInterval() const
{
    return std::chrono::seconds(resolve<int>("splunk.flush_interval_sec", 5));
}

template<typename T>
T SplunkLogSettings::resolve(const std::string& configKey, T defaultValue) const
{
    std::string fullKey = "splunk." + configKey;
    std::string envVarName = prefix_.empty()
        ? toEnvName(configKey)
        : prefix_ + "_" + toEnvName(configKey);

    const char* envValue = std::getenv(envVarName.c_str());
    if (envValue)
    {
        if constexpr (std::is_same_v<T, std::string>)
        {
            return std::string(envValue);
        }
        else if constexpr (std::is_same_v<T, int>)
        {
            return std::stoi(envValue);
        }
        else if constexpr (std::is_same_v<T, size_t>)
        {
            return std::stoul(envValue);
        }
        else
        {
            return T();
        }
    }

    if (env_)
    {
        try
        {
            return env_->get<T>(fullKey);
        }
        catch (...)
        {
            return defaultValue;
        }
    }

    return defaultValue;
}
