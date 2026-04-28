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
/**
 * @file IServerSettings.hpp
 * @brief Server settings interface
 * @author Anton Tobolkin
 */

/**
 * @class IServerSettings
 * @brief Interface for server configuration
 *
 * Provides read-only access to all server settings.
 * Implementations may load from environment variables, config files, or defaults.
 */
class IServerSettings {
public:
    virtual ~IServerSettings() = default;

    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
    virtual size_t getMaxRequestBodySize() const = 0;

    /**
     * @brief Timeout for reading the first HTTP request on a new connection
     * @return Read timeout in milliseconds
     */
    virtual std::chrono::milliseconds getReadTimeout() const = 0;

    /**
     * @brief Timeout for writing HTTP response
     * @return Write timeout in milliseconds
     */
    virtual std::chrono::milliseconds getWriteTimeout() const = 0;

    /**
     * @brief Idle timeout for keep-alive connections
     *
     * After sending a response, the server waits up to this duration
     * for the next request on the same connection. If no request arrives,
     * the connection is closed. Default: 5000ms.
     *
     * @return Keep-alive idle timeout in milliseconds
     */
    virtual std::chrono::milliseconds getKeepAliveTimeout() const = 0;

    virtual size_t getMaxConnections() const = 0;
    virtual size_t getMaxRequestsPerConnection() const = 0;
};
