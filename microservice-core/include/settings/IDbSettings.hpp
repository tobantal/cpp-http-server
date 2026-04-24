#pragma once

#include <string>
#include <cstddef>

/**
 * @file IDbSettings.hpp
 * @brief Interface for database connection settings
 * @author Anton Tobolkin
 */

/**
 * @class IDbSettings
 * @brief Database connection settings interface
 *
 * Provides PostgreSQL connection parameters and pool sizing.
 * Implementations read from ENV, config files, or other sources.
 * Follows the same pattern as IServerSettings.
 */
class IDbSettings {
public:
    virtual ~IDbSettings() = default;

    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
    virtual std::string getName() const = 0;
    virtual std::string getUser() const = 0;
    virtual std::string getPassword() const = 0;
    virtual size_t getMinConnections() const = 0;
    virtual size_t getMaxConnections() const = 0;

    /**
     * @brief PostgreSQL connection string for libpqxx
     * @return Connection string in "host=... port=... dbname=... user=... password=..." format
     */
    virtual std::string getConnectionString() const = 0;
};