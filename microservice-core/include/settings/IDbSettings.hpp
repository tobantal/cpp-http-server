#pragma once
#include <string>

/**
 * @file IDbSettings.hpp
 * @brief Интерфейс настроек подключения к БД
 * @author Anton Tobolkin
 */

/**
 * @interface IDbSettings
 * @brief Интерфейс для получения параметров подключения к базе данных
 */
class IDbSettings
{
public:
    virtual ~IDbSettings() = default;
    
    /**
     * @brief Получить хост БД
     */
    virtual std::string getHost() const = 0;
    
    /**
     * @brief Получить порт БД
     */
    virtual int getPort() const = 0;
    
    /**
     * @brief Получить имя базы данных
     */
    virtual std::string getName() const = 0;
    
    /**
     * @brief Получить имя пользователя
     */
    virtual std::string getUser() const = 0;
    
    /**
     * @brief Получить пароль
     */
    virtual std::string getPassword() const = 0;
};