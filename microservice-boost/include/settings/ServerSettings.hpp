#pragma once

#include <memory>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include "settings/IServerSettings.hpp"
#include "ports/output/IEnvironment.hpp"

/**
 * @file ServerSettings.hpp
 * @brief Server settings implementation
 * @author Anton Tobolkin
 */

/**
 * @class ServerSettings
 * @brief Server configuration loaded from environment and config
 *
 * Reads settings from environment variables (prefixed with SERVER_) or config.json.
 * Environment variables take precedence over config.json values.
 *
 * <b>Keep-alive idle timeout</b>: After sending a response on a keep-alive connection,
 * the server waits for the next request up to keepAliveTimeoutMs before closing.
 * Default: 5000ms (5s). Similar to Node.js keepAliveTimeout (5s) or Tomcat keepAliveTimeout (20s).
 */
class ServerSettings : public IServerSettings {
private:
    std::string host_;
    int port_;
    size_t maxRequestBodySize_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;
    std::chrono::milliseconds keepAliveTimeout_;
    size_t maxConnections_;
    size_t maxRequestsPerConnection_;

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue);
    static size_t getEnvOrDefaultSize(const char* name, size_t defaultValue);

    static constexpr size_t kDefaultMaxRequestBodySize = 16 * 1024 * 1024;
    static constexpr int kDefaultReadTimeoutMs = 30000;
    static constexpr int kDefaultWriteTimeoutMs = 30000;
    static constexpr int kDefaultKeepAliveTimeoutMs = 5000;
    static constexpr size_t kDefaultMaxConnections = 1024;
    static constexpr size_t kDefaultMaxRequestsPerConnection = 100;

public:
    /**
     * @brief Construct ServerSettings from environment
     * @param env Environment interface
     */
    explicit ServerSettings(std::shared_ptr<IEnvironment> env);

    std::string getHost() const override;
    int getPort() const override;
    size_t getMaxRequestBodySize() const override;
    std::chrono::milliseconds getReadTimeout() const override;
    std::chrono::milliseconds getWriteTimeout() const override;
    std::chrono::milliseconds getKeepAliveTimeout() const override;
    size_t getMaxConnections() const override;
    size_t getMaxRequestsPerConnection() const override;
};