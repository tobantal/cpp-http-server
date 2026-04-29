#pragma once

#include "ports/input/IJsonToEnvConverter.hpp"
#include "ports/output/IEnvironment.hpp"
#include "adapters/secondary/Environment.hpp"
#include "domain/error/ConvertError.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <any>
#include <map>

/**
 * @file JsonToEnvConverter.hpp
 * @brief JSON string to IEnvironment converter implementation
 * @author Anton Tobolkin
 */

/**
 * @class JsonToEnvConverter
 * @brief Converts JSON string to IEnvironment using nlohmann::json
 */
class JsonToEnvConverter : public IJsonToEnvConverter
{
public:
    /**
     * @brief Convert JSON string to IEnvironment
     * @param input JSON string
     * @return shared_ptr to IEnvironment with parsed properties
     * @throws ConvertError if JSON is invalid or cannot be converted
     */
    std::shared_ptr<IEnvironment> convert(const std::string& input) const override
    {
        try
        {
            auto json = nlohmann::json::parse(input);
            auto env = std::make_shared<Environment>();

            for (auto& [key, value] : json.items())
            {
                env->setProperty(key, toAny(value));
            }

            return env;
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw ConvertError("Invalid JSON", e.what());
        }
    }

private:
    std::any toAny(const nlohmann::json& value) const
    {
        switch (value.type())
        {
            case nlohmann::json::value_t::string:
                return std::any(value.get<std::string>());
            case nlohmann::json::value_t::number_integer:
                return std::any(value.get<int>());
            case nlohmann::json::value_t::number_unsigned:
                return std::any(static_cast<int>(value.get<unsigned int>()));
            case nlohmann::json::value_t::number_float:
                return std::any(value.get<double>());
            case nlohmann::json::value_t::boolean:
                return std::any(value.get<bool>());
            case nlohmann::json::value_t::null:
                return std::any();
            default:
                return std::any(value.dump());
        }
    }
};