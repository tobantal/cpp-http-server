#pragma once

#include <any>
#include <string>
#include <stdexcept>

/**
 * @file IEnvironment.hpp
 * @brief Interface for universal environment object
 * @author Anton Tobolkin
 */

/**
 * @class IEnvironment
 * @brief Interface for managing application configuration
 */
class IEnvironment
{
public:
    virtual ~IEnvironment() = default;

    /**
     * @brief Get environment property value
     * @param key Property key (name)
     * @return Property value (may be of any type)
     * @throws std::runtime_error If property not found
     */
    virtual std::any getProperty(const std::string& key) const = 0;

    /**
     * @brief Set environment property value
     * @param key Property key (name)
     * @param value New property value
     */
    virtual void setProperty(const std::string& key, const std::any& value) = 0;

    /**
     * @brief Type-safe property getter
     * @tparam T Expected value type
     * @param key Property key
     * @return Property value cast to type T
     * @throws std::bad_any_cast If type does not match
     */
    template<typename T>
    T get(const std::string& key) const
    {
        return std::any_cast<T>(getProperty(key));
    }

    /**
     * @brief Type-safe getter with default value
     * @tparam T Expected value type
     * @param key Property key
     * @param defaultValue Default value if key not found
     * @return Property value or defaultValue
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const
    {
        try
        {
            return std::any_cast<T>(getProperty(key));
        }
        catch (const std::exception&)
        {
            return defaultValue;
        }
    }
};
