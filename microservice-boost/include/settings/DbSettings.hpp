#pragma once

#include "settings/IDbSettings.hpp"
#include "ports/output/IEnvironment.hpp"
#include <cstdlib>
#include <string>
#include <memory>
#include <type_traits>

/**
 * @file DbSettings.hpp
 * @brief PostgreSQL connection settings with 3-tier fallback: ENV - config.json - default
 * @author Anton Tobolkin
 */

/**
 * @class DbSettings
 * @brief Database settings with ENV → config.json → default fallback
 *
 * Naming: config key "auth.db.host" → ENV var "AUTH_DB_HOST" (uppercase, dots → underscores)
 *
 * resolve() method:
 * 1. Check ENV via std::getenv(envVarName)
 * 2. If not set, check config.json via env_->get<T>(configKey)
 * 3. If not set, use default value
 *
 * @example
 *   DbSettings settings(env, "auth");
 *   // reads AUTH_DB_HOST, AUTH_DB_PORT, etc.
 *   // config keys: auth.db.host, auth.db.port, auth.db.name...
 *   // defaults: auth-postgres:5432/auth_db/auth_user
 */
class DbSettings : public IDbSettings {
public:
    /**
     * @param env IEnvironment pointer (may be nullptr)
     * @param prefix ENV prefix (AUTH, BROKER, TRADING)
     */
    DbSettings(std::shared_ptr<IEnvironment> env, const std::string& prefix);

    ~DbSettings() override = default;

    std::string getHost() const override;
    int getPort() const override;
    std::string getName() const override;
    std::string getUser() const override;
    std::string getPassword() const override;
    size_t getMinConnections() const override;
    size_t getMaxConnections() const override;

    std::string getConnectionString() const override;

private:
    std::shared_ptr<IEnvironment> env_;
    std::string prefix_;

    template<typename T>
    T resolve(const std::string& configKey, T defaultValue) const;

    static std::string toEnvName(const std::string& configKey);
    static std::string toLower(const std::string& s);
};

template<typename T>
T DbSettings::resolve(const std::string& configKey, T defaultValue) const
{
    std::string fullKey = prefix_.empty() ? configKey : toLower(prefix_) + "." + configKey;
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
