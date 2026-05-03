#include "settings/DbSettings.hpp"
#include <algorithm>
#include <cctype>
#include <stdexcept>

DbSettings::DbSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix)
    : env_(env), prefix_(prefix)
{
}

std::string DbSettings::toEnvName(const std::string& configKey)
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

std::string DbSettings::toLower(const std::string& s)
{
    std::string result = s;
    for (auto& c : result)
    {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string DbSettings::getHost() const
{
    std::string defaultHost = prefix_ + "-postgres";
    return resolve<std::string>("db.host", defaultHost);
}

int DbSettings::getPort() const
{
    return resolve<int>("db.port", 5432);
}

std::string DbSettings::getName() const
{
    std::string defaultName = toLower(prefix_) + "_db";
    return resolve<std::string>("db.name", defaultName);
}

std::string DbSettings::getUser() const
{
    std::string defaultUser = toLower(prefix_) + "_user";
    return resolve<std::string>("db.user", defaultUser);
}

std::string DbSettings::getPassword() const
{
    std::string defaultPassword = toLower(prefix_) + "_secret_password";
    return resolve<std::string>("db.password", defaultPassword);
}

size_t DbSettings::getMinConnections() const
{
    return resolve<size_t>("db.pool_min", 2);
}

size_t DbSettings::getMaxConnections() const
{
    return resolve<size_t>("db.pool_max", 10);
}

std::string DbSettings::getConnectionString() const
{
    return "host=" + getHost() + " port=" + std::to_string(getPort()) +
           " dbname=" + getName() + " user=" + getUser() + " password=" + getPassword();
}
