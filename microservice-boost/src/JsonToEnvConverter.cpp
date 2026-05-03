#include "adapters/primary/JsonToEnvConverter.hpp"
#include "domain/error/ConvertError.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <any>

std::shared_ptr<IEnvironment> JsonToEnvConverter::convert(const std::string& input) const {
    try
    {
        auto json = nlohmann::json::parse(input);
        auto env = std::make_shared<Environment>();

        for (auto& [key, value] : json.items())
        {
            if (value.is_null())
            {
                continue;
            }
            env->setProperty(key, toAny(value));
        }

        return env;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        throw ConvertError("Invalid JSON", e.what());
    }
}

std::any JsonToEnvConverter::toAny(const nlohmann::json& value) const {
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
        case nlohmann::json::value_t::array:
            return std::any(arrayToString(value));
        default:
            return std::any(value.dump());
    }
}

std::string JsonToEnvConverter::arrayToString(const nlohmann::json& arr) const
{
    if (arr.empty())
    {
        return "";
    }
    bool allStrings = std::all_of(arr.begin(), arr.end(),
        [](const nlohmann::json& el) { return el.is_string(); });
    std::string result;
    for (auto it = arr.begin(); it != arr.end(); ++it)
    {
        if (it != arr.begin()) result += ",";
        if (allStrings)
        {
            result += it->get<std::string>();
        }
        else
        {
            result += it->dump();
        }
    }
    return result;
}