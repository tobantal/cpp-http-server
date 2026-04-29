/**
 * @file Environment.cpp
 * @brief Environment implementation
 * @author Anton Tobolkin
 */

#include "adapters/secondary/Environment.hpp"

/**
 * @brief Get property value by key
 * @param key Property key
 * @return Property value as std::any
 * @throws std::runtime_error if key not found
 */
std::any Environment::getProperty(const std::string& key) const
{
    auto it = properties_.find(key);
    if (it == properties_.end())
    {
        throw std::runtime_error("Property not found: " + key);
    }
    return it->second;
}

/**
 * @brief Set property value
 * @param key Property key
 * @param value New value
 */
void Environment::setProperty(const std::string& key, const std::any& value)
{
    properties_[key] = value;
}