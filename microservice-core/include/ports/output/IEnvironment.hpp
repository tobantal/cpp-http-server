#pragma once

#include <any>
#include <string>
#include <optional>
#include "domain/error/ConvertError.hpp"

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
     * @throws ConvertError If property not found
     */
    virtual std::any getProperty(const std::string& key) const = 0;

    /**
     * @brief Check if property exists
     * @param key Property key
     * @return true if property exists
     */
    virtual bool hasProperty(const std::string& key) const = 0;

    /**
     * @brief Set environment property value
     * @param key Property key (name)
     * @param value New property value
     */
    virtual void setProperty(const std::string& key, const std::any& value) = 0;

    /**
     * @brief Type-safe getter for required property
     * @tparam T Expected value type
     * @param key Property key
     * @return Property value cast to type T
     * @throws ConvertError If key not found or type does not match
     */
    template<typename T>
    T get(const std::string& key) const
    {
        try
        {
            return std::any_cast<T>(getProperty(key));
        }
        catch (const ConvertError&)
        {
            throw;
        }
        catch (const std::bad_any_cast&)
        {
            throw ConvertError("Type mismatch for key: " + key);
        }
    }

    /**
     * @brief Type-safe getter for optional property
     * @tparam T Expected value type
     * @param key Property key
     * @return optional with value if key exists and type matches, nullopt if key absent
     * @throws ConvertError If key exists but type does not match
     */
    template<typename T>
    std::optional<T> get_optional(const std::string& key) const
    {
        if (!hasProperty(key))
        {
            return std::nullopt;
        }
        try
        {
            return std::any_cast<T>(getProperty(key));
        }
        catch (const std::bad_any_cast&)
        {
            throw ConvertError("Type mismatch for key: " + key);
        }
    }
};