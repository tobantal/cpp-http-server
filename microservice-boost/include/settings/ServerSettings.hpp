#pragma once

#include <memory>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include <chrono>
#include "settings/IServerSettings.hpp"
#include "IEnvironment.hpp"

class ServerSettings : public IServerSettings {
private:
    std::string host_;
    int port_;
    size_t maxRequestBodySize_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : defaultValue;
    }

    static size_t getEnvOrDefaultSize(const char* name, size_t defaultValue) {
        const char* value = std::getenv(name);
        if (value) {
            try {
                return std::stoul(value);
            } catch (...) {
                return defaultValue;
            }
        }
        return defaultValue;
    }

    static constexpr size_t kDefaultMaxRequestBodySize = 1048576;
    static constexpr int kDefaultReadTimeoutMs = 30000;
    static constexpr int kDefaultWriteTimeoutMs = 30000;

public:
    explicit ServerSettings(std::shared_ptr<IEnvironment> env)
        : host_("0.0.0.0"), port_(8080), maxRequestBodySize_(kDefaultMaxRequestBodySize),
          readTimeout_(kDefaultReadTimeoutMs), writeTimeout_(kDefaultWriteTimeoutMs)
    {
        host_ = getEnvOrDefault("SERVER_HOST", "");
        port_ = 0;

        if (host_.empty()) {
            try {
                host_ = env->get<std::string>("server.host");
            } catch (...) {
                host_ = "0.0.0.0";
            }
        }

        const char* portEnv = std::getenv("SERVER_PORT");
        if (portEnv) {
            try {
                port_ = std::stoi(portEnv);
            } catch (...) {
                port_ = 8080;
            }
        } else {
            try {
                port_ = env->get<int>("server.port");
            } catch (...) {
                port_ = 8080;
            }
        }

        size_t envMaxBody = getEnvOrDefaultSize("SERVER_MAX_REQUEST_BODY_SIZE", 0);
        if (envMaxBody > 0) {
            maxRequestBodySize_ = envMaxBody;
        } else {
            try {
                maxRequestBodySize_ = env->get<size_t>("server.maxRequestBodySize");
            } catch (...) {
                maxRequestBodySize_ = kDefaultMaxRequestBodySize;
            }
        }

        size_t envReadTimeout = getEnvOrDefaultSize("SERVER_READ_TIMEOUT_MS", 0);
        if (envReadTimeout > 0) {
            readTimeout_ = std::chrono::milliseconds(envReadTimeout);
        } else {
            try {
                readTimeout_ = std::chrono::milliseconds(env->get<int>("server.readTimeoutMs"));
            } catch (...) {
                readTimeout_ = std::chrono::milliseconds(kDefaultReadTimeoutMs);
            }
        }

        size_t envWriteTimeout = getEnvOrDefaultSize("SERVER_WRITE_TIMEOUT_MS", 0);
        if (envWriteTimeout > 0) {
            writeTimeout_ = std::chrono::milliseconds(envWriteTimeout);
        } else {
            try {
                writeTimeout_ = std::chrono::milliseconds(env->get<int>("server.writeTimeoutMs"));
            } catch (...) {
                writeTimeout_ = std::chrono::milliseconds(kDefaultWriteTimeoutMs);
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
};
