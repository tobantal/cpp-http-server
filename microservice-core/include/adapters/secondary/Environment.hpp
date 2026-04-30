#pragma once

#include "ports/output/IEnvironment.hpp"
#include <map>
#include <any>
#include <string>
#include <stdexcept>

/**
 * @file Environment.hpp
 * @brief Concrete implementation of IEnvironment
 * @author Anton Tobolkin
 */

/**
 * @class Environment
 * @brief IEnvironment implementation based on std::map
 */
class Environment : public IEnvironment
{
public:
    /**
     * @brief Get property value
     * @param key Property key
     * @return Property value
     * @throws std::runtime_error If property not found
     */
    std::any getProperty(const std::string& key) const override;

    /**
     * @brief Set property value
     * @param key Property key
     * @param value New value
     */
    void setProperty(const std::string& key, const std::any& value) override;

private:
    std::map<std::string, std::any> properties_;
};