#pragma once

#include "settings/IDbSettings.hpp"
#include <cstdlib>
#include <string>

/**
 * @file DbSettings.hpp
 * @brief PostgreSQL connection settings from environment variables
 * @author Anton Tobolkin
 */

/**
 * @class DbSettings
 * @brief Database settings loaded from ENV with parameterized prefix
 *
 * Reads <PREFIX>_DB_HOST, <PREFIX>_DB_PORT, <PREFIX>_DB_NAME,
 * <PREFIX>_DB_USER, <PREFIX>_DB_PASSWORD, <PREFIX>_DB_POOL_MIN,
 * <PREFIX>_DB_POOL_MAX. Falls back to reasonable defaults.
 *
 * Follows the same pattern as ServerSettings.
 *
 * @example
 *   DbSettings settings("AUTH");
 *   // reads AUTH_DB_HOST, AUTH_DB_PORT, etc.
 *   // defaults: AUTH-postgres:5432/auth_db/auth_user
 */
class DbSettings : public IDbSettings {
public:
    /**
     * @param prefix ENV prefix (AUTH, BROKER, TRADING)
     * @param defaultHost Default host (for K8s: "<prefix>-postgres")
     */
    explicit DbSettings(const std::string& prefix,
                        const std::string& defaultHost = "")
        : prefix_(prefix)
    {
        std::string hostDefault = defaultHost.empty()
            ? (prefix + "-postgres") : defaultHost;

        host_ = getEnvOrDefault((prefix + "_DB_HOST").c_str(), hostDefault);
        port_ = std::stoi(getEnvOrDefault((prefix + "_DB_PORT").c_str(), "5432"));
        name_ = getEnvOrDefault((prefix + "_DB_NAME").c_str(), toLower(prefix) + "_db");
        user_ = getEnvOrDefault((prefix + "_DB_USER").c_str(), toLower(prefix) + "_user");
        password_ = getEnvOrDefault((prefix + "_DB_PASSWORD").c_str(),
                                    toLower(prefix) + "_secret_password");
        minConnections_ = std::stoul(getEnvOrDefault((prefix + "_DB_POOL_MIN").c_str(), "2"));
        maxConnections_ = std::stoul(getEnvOrDefault((prefix + "_DB_POOL_MAX").c_str(), "10"));
    }

    ~DbSettings() override = default;

    std::string getHost() const override { return host_; }
    int getPort() const override { return port_; }
    std::string getName() const override { return name_; }
    std::string getUser() const override { return user_; }
    std::string getPassword() const override { return password_; }
    size_t getMinConnections() const override { return minConnections_; }
    size_t getMaxConnections() const override { return maxConnections_; }

    std::string getConnectionString() const override {
        return "host=" + host_ + " port=" + std::to_string(port_) +
               " dbname=" + name_ + " user=" + user_ + " password=" + password_;
    }

private:
    std::string prefix_;
    std::string host_;
    int port_;
    std::string name_;
    std::string user_;
    std::string password_;
    size_t minConnections_;
    size_t maxConnections_;

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : defaultValue;
    }

    static std::string toLower(const std::string& s) {
        std::string result = s;
        for (auto& c : result) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        return result;
    }
};