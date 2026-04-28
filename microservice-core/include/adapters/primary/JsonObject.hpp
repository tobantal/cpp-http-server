#pragma once

#include "ports/output/IEnvironment.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <map>

/**
 * @file JsonObject.hpp
 * @brief JSON object wrapper providing type-safe access to JSON data
 * @author AI-Coder
 */

/**
 * @class JsonObject
 * @brief Wrapper around nlohmann::json that implements IEnvironment
 *
 * Parses JSON string in constructor and stores values as IEnvironment properties.
 * Provides type-safe access via Environment::get<T>() and set<T>().
 *
 * Example:
 * @code
 * JsonObject obj(R"({"username": "admin", "age": 30})");
 * auto name = obj.get<std::string>("username");
 * auto age = obj.get<int>("age");
 * @endcode
 */
class JsonObject : public IEnvironment
{
public:
    /**
     * @brief Construct JsonObject from JSON string
     * @param body JSON string to parse
     * @throws nlohmann::json::parse_error if JSON is invalid
     */
    explicit JsonObject(const std::string& body);

    /**
     * @brief Serialize to JSON string
     * @return JSON string representation
     */
    std::string toJson();

    /**
     * @brief Get property value
     * @param key Property key
     * @return Property value or empty any if not found
     */
    std::any getProperty(const std::string& key) const override;

    /**
     * @brief Set property value
     * @param key Property key
     * @param value Property value
     */
    void setProperty(const std::string& key, const std::any& value) override;

private:
    nlohmann::json json_;
    std::map<std::string, std::any> properties_;
};