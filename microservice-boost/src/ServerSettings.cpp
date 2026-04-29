#include "settings/ServerSettings.hpp"

#include <utility>

namespace {

std::string getEnvOrDefault(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

size_t getEnvOrDefaultSize(const char* name, size_t defaultValue) {
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

}

ServerSettings::ServerSettings(std::shared_ptr<IEnvironment> env)
    : host_("0.0.0.0"), port_(8080), maxRequestBodySize_(kDefaultMaxRequestBodySize),
      readTimeout_(kDefaultReadTimeoutMs), writeTimeout_(kDefaultWriteTimeoutMs),
      keepAliveTimeout_(std::chrono::milliseconds(5000)),
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

    size_t envMaxConn = getEnvOrDefaultSize("SERVER_MAX_CONNECTIONS", 0);
    if (envMaxConn > 0) {
        maxConnections_ = envMaxConn;
    } else {
        try {
            maxConnections_ = env->get<size_t>("server.maxConnections");
        } catch (const std::exception&) {
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

std::string ServerSettings::getHost() const {
    return host_;
}

int ServerSettings::getPort() const {
    return port_;
}

size_t ServerSettings::getMaxRequestBodySize() const {
    return maxRequestBodySize_;
}

std::chrono::milliseconds ServerSettings::getReadTimeout() const {
    return readTimeout_;
}

std::chrono::milliseconds ServerSettings::getWriteTimeout() const {
    return writeTimeout_;
}

std::chrono::milliseconds ServerSettings::getKeepAliveTimeout() const {
    return keepAliveTimeout_;
}

size_t ServerSettings::getMaxConnections() const {
    return maxConnections_;
}

size_t ServerSettings::getMaxRequestsPerConnection() const {
    return maxRequestsPerConnection_;
}