#pragma once

#include <string>

/**
 * @file IServerSettings.hpp
 * @brief Интерфейс настроек сервера
 * @author Anton Tobolkin
 */
class IServerSettings {
public:
    virtual ~IServerSettings() = default;
    virtual std::string getHost() const = 0;
    virtual int getPort() const = 0;
};