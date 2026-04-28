#include "adapters/primary/JsonObject.hpp"
#include <stdexcept>

JsonObject::JsonObject(const std::string& body)
{
    json_ = nlohmann::json::parse(body);
    for (auto& [key, value] : json_.items())
    {
        if (value.is_string())
        {
            setProperty(key, value.get<std::string>());
        }
        else if (value.is_number_integer())
        {
            setProperty(key, value.get<int>());
        }
        else if (value.is_number())
        {
            setProperty(key, value.get<double>());
        }
        else if (value.is_boolean())
        {
            setProperty(key, value.get<bool>());
        }
        else if (value.is_null())
        {
            setProperty(key, std::any());
        }
        else
        {
            setProperty(key, value.dump());
        }
    }
}

std::string JsonObject::toJson()
{
    return json_.dump();
}

std::any JsonObject::getProperty(const std::string& key) const
{
    auto it = properties_.find(key);
    if (it == properties_.end())
    {
        return std::any();
    }
    return it->second;
}

void JsonObject::setProperty(const std::string& key, const std::any& value)
{
    properties_[key] = value;
}