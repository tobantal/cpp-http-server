#pragma once

#include <string>
#include <cstddef>
#include <chrono>

/**
 * @file IServerSettings.hpp
 * @brief Server settings interface
 * @author Anton Tobolkin
 */

/**
 * @class IServerSettings
 * @brief Interface for server configuration
 */
class IServerSettings {
public:
    virtual ~IServerSettings() = default;

    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
    virtual size_t getMaxRequestBodySize() const = 0;
    virtual std::chrono::milliseconds getReadTimeout() const = 0;
    virtual std::chrono::milliseconds getWriteTimeout() const = 0;
    virtual size_t getMaxConnections() const = 0;
};
