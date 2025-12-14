#pragma once
#include "settings/IDbSettings.hpp"
#include "IEnvironment.hpp"
#include <memory>
#include <string>

/**
 * @file DbSettings.hpp
 * @brief Адаптер настроек подключения к БД из Environment
 * @author Anton Tobolkin
 */

/**
 * @class DbSettings
 * @brief Реализация IDbSettings, получает данные из Environment
 */
class DbSettings : public IDbSettings
{
public:
    /**
     * @brief Конструктор с валидацией всех полей
     * @param env Окружение с настройками
     * @throws EnvironmentLoadException если настройки не валидны
     */
    explicit DbSettings(std::shared_ptr<IEnvironment> env);
    
    std::string getHost() const override;
    int getPort() const override;
    std::string getName() const override;
    std::string getUser() const override;
    std::string getPassword() const override;

private:
    std::string host_;
    int port_;
    std::string name_;
    std::string user_;
    std::string password_;
};