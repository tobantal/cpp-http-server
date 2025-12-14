#pragma once

#include <any>
#include <string>
#include <stdexcept>

/**
 * @file IEnvironment.hpp
 * @brief Интерфейс универсального объекта окружения
 * @author Anton Tobolkin
 */

/**
 * @class IEnvironment
 * @brief Интерфейс для управления конфигурацией приложения
 * 
 * Реализует паттерн "Универсальный объект" для хранения
 * конфигурации приложения с произвольным набором свойств.
 * 
 * Использование std::any обеспечивает хранение свойств
 * различных типов данных без необходимости создания
 * специализированных классов конфигурации.
 */
class IEnvironment
{
public:
    virtual ~IEnvironment() = default;

    /**
     * @brief Получить значение свойства окружения
     * @param key Ключ (название) свойства
     * @return Значение свойства (может быть любого типа)
     * @throws std::runtime_error Если свойство не найдено
     */
    virtual std::any getProperty(const std::string& key) const = 0;

    /**
     * @brief Установить значение свойства окружения
     * @param key Ключ (название) свойства
     * @param value Новое значение свойства
     * @throws std::runtime_error Если операция не поддерживается
     */
    virtual void setProperty(const std::string& key, const std::any& value) = 0;

    /**
     * @brief Типобезопасный геттер свойства
     * @tparam T Ожидаемый тип значения
     * @param key Ключ свойства
     * @return Значение свойства приведенное к типу T
     * @throws std::bad_any_cast Если тип не совпадает
     */
    template<typename T>
    T get(const std::string& key) const
    {
        return std::any_cast<T>(getProperty(key));
    }

    /**
     * @brief Типобезопасный геттер с дефолтным значением
     * @tparam T Ожидаемый тип значения
     * @param key Ключ свойства
     * @param defaultValue Значение по умолчанию если ключ не найден
     * @return Значение свойства или defaultValue
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const
    {
        try
        {
            return std::any_cast<T>(getProperty(key));
        }
        catch (...)
        {
            return defaultValue;
        }
    }
};