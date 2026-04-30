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

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : defaultValue;
    }

    static size_t getEnvOrDefaultSize(const char* name, size_t defaultValue) {
        const char* value = std::getenv(name);
        if (value) {
            try {
                return std::stoul(value);
            } catch (const std::exception&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

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
    explicit ServerSettings(std::shared_ptr<IEnvironment> env)
        : host_("0.0.0.0"), port_(8080), maxRequestBodySize_(kDefaultMaxRequestBodySize),
           readTimeout_(kDefaultReadTimeoutMs), writeTimeout_(kDefaultWriteTimeoutMs),
           keepAliveTimeout_(kDefaultKeepAliveTimeoutMs),
           maxConnections_(kDefaultMaxConnections),
           maxRequestsPerConnection_(kDefaultMaxRequestsPerConnection)
    {
        host_ = getEnvOrDefault("SERVER_HOST", "");
        port_ = 0;

        if (host_.empty()) {
            try {
                host_ = env->get<std::string>("server.host");
            } catch (const std::exception&) {
                host_ = "0.0.0.0";
            }
        }

        const char* portEnv = std::getenv("SERVER_PORT");
        if (portEnv) {
            try {
                port_ = std::stoi(portEnv);
            } catch (const std::exception&) {
                port_ = 8080;
            }
        } else {
            try {
                port_ = env->get<int>("server.port");
            } catch (const std::exception&) {
                port_ = 8080;
            }
        }

        size_t envMaxBody = getEnvOrDefaultSize("SERVER_MAX_REQUEST_BODY_SIZE", 0);
        if (envMaxBody > 0) {
            maxRequestBodySize_ = envMaxBody;
        } else {
            try {
                maxRequestBodySize_ = env->get<size_t>("server.maxRequestBodySize");
            } catch (const std::exception&) {
                maxRequestBodySize_ = kDefaultMaxRequestBodySize;
            }
        }

        size_t envReadTimeout = getEnvOrDefaultSize("SERVER_READ_TIMEOUT_MS", 0);
        if (envReadTimeout > 0) {
            readTimeout_ = std::chrono::milliseconds(envReadTimeout);
        } else {
            try {
                readTimeout_ = std::chrono::milliseconds(env->get<int>("server.readTimeoutMs"));
            } catch (const std::exception&) {
                readTimeout_ = std::chrono::milliseconds(kDefaultReadTimeoutMs);
            }
        }

        size_t envWriteTimeout = getEnvOrDefaultSize("SERVER_WRITE_TIMEOUT_MS", 0);
        if (envWriteTimeout > 0) {
            writeTimeout_ = std::chrono::milliseconds(envWriteTimeout);
        } else {
            try {
                writeTimeout_ = std::chrono::milliseconds(env->get<int>("server.writeTimeoutMs"));
            } catch (const std::exception&) {
                writeTimeout_ = std::chrono::milliseconds(kDefaultWriteTimeoutMs);
            }
        }

        size_t envKeepAliveTimeout = getEnvOrDefaultSize("SERVER_KEEP_ALIVE_TIMEOUT_MS", 0);
        if (envKeepAliveTimeout > 0) {
            keepAliveTimeout_ = std::chrono::milliseconds(envKeepAliveTimeout);
        } else {
            try {
                keepAliveTimeout_ = std::chrono::milliseconds(env->get<int>("server.keepAliveTimeoutMs"));
            } catch (const std::exception&) {
                keepAliveTimeout_ = std::chrono::milliseconds(kDefaultKeepAliveTimeoutMs);
            }
        }

        size_t envMaxConn = getEnvOrDefaultSize("SERVER_MAX_CONNECTIONS", 0);
        if (envMaxConn > 0) {
            maxConnections_ = envMaxConn;
        } else {
            try {
                maxConnections_ = env->get<size_t>("server.maxConnections");
            }             catch (const std::exception&) {
                maxConnections_ = kDefaultMaxConnections;
            }
        }

        size_t envMaxReqs = getEnvOrDefaultSize("SERVER_MAX_REQUESTS_PER_CONNECTION", 0);
        if (envMaxReqs > 0) {
            maxRequestsPerConnection_ = envMaxReqs;
        } else {
            try {
                maxRequestsPerConnection_ = env->get<size_t>("server.maxRequestsPerConnection");
            } catch (const std::exception&) {
                maxRequestsPerConnection_ = kDefaultMaxRequestsPerConnection;
            }
        }
    }

    std::string getHost() const override {
        return host_;
    }

    int getPort() const override {
        return port_;
    }

    size_t getMaxRequestBodySize() const override {
        return maxRequestBodySize_;
    }

    std::chrono::milliseconds getReadTimeout() const override {
        return readTimeout_;
    }

    std::chrono::milliseconds getWriteTimeout() const override {
        return writeTimeout_;
    }

    std::chrono::milliseconds getKeepAliveTimeout() const override {
        return keepAliveTimeout_;
    }

    size_t getMaxConnections() const override {
        return maxConnections_;
    }

    size_t getMaxRequestsPerConnection() const override {
        return maxRequestsPerConnection_;
    }
};
