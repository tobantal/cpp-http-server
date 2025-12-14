#pragma once

#include "IEnvironment.hpp"
#include <map>
#include <any>
#include <string>
#include <stdexcept>

/**
 * @file Environment.hpp
 * @brief Реализация универсального объекта окружения
 * @author Anton Tobolkin
 */

/**
 * @class Environment
 * @brief Конкретная реализация IEnvironment на основе std::map
 * 
 * Хранит конфигурацию приложения в виде пар ключ-значение.
 * Используется для хранения параметров окружения: хост, порт,
 * пути к сертификатам, строки подключения к БД и т.д.
 */
class Environment : public IEnvironment
{
public:
    /**
     * @brief Получить значение свойства
     * @param key Ключ свойства
     * @return Значение свойства
     * @throws std::runtime_error Если свойство не найдено
     */
    std::any getProperty(const std::string& key) const override
    {
        auto it = properties_.find(key);
        if (it == properties_.end())
        {
            throw std::runtime_error("Property not found: " + key);
        }
        return it->second;
    }

    /**
     * @brief Установить значение свойства
     * @param key Ключ свойства
     * @param value Новое значение
     */
    void setProperty(const std::string& key, const std::any& value) override
    {
        properties_[key] = value;
    }

private:
    std::map<std::string, std::any> properties_;
};