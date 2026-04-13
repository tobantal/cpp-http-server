#pragma once

#include <memory>
#include <string>
#include <cstdlib>
#include <stdexcept>
#include "settings/IServerSettings.hpp"
#include "IEnvironment.hpp"

class ServerSettings : public IServerSettings {
private:
    std::string host_;
    int port_;
    size_t maxRequestBodySize_;

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

public:
    explicit ServerSettings(std::shared_ptr<IEnvironment> env)
        : host_("0.0.0.0"), port_(8080), maxRequestBodySize_(kDefaultMaxRequestBodySize)
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
};
