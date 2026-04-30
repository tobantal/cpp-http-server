#include "settings/DbSettings.hpp"

std::string DbSettings::getHost() const {
    return host_;
}

int DbSettings::getPort() const {
    return port_;
}

std::string DbSettings::getName() const {
    return name_;
}

std::string DbSettings::getUser() const {
    return user_;
}

std::string DbSettings::getPassword() const {
    return password_;
}

size_t DbSettings::getMinConnections() const {
    return minConnections_;
}

size_t DbSettings::getMaxConnections() const {
    return maxConnections_;
}

std::string DbSettings::getConnectionString() const {
    return "host=" + host_ + " port=" + std::to_string(port_) +
           " dbname=" + name_ + " user=" + user_ + " password=" + password_;
}

std::string DbSettings::getEnvOrDefault(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}

std::string DbSettings::toLower(const std::string& s) {
    std::string result = s;
    for (auto& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}