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

    static std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
        const char* value = std::getenv(name);
        return value ? std::string(value) : defaultValue;
    }

public:
    explicit ServerSettings(std::shared_ptr<IEnvironment> env)
        : host_("0.0.0.0"), port_(8080)
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
    }

    std::string getHost() const override {
        return host_;
    }

    int getPort() const override {
        return port_;
    }
};
