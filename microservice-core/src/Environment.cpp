#include "adapters/secondary/Environment.hpp"
#include "domain/error/ConvertError.hpp"
#include <map>
#include <any>
#include <string>

std::any Environment::getProperty(const std::string& key) const {
    auto it = properties_.find(key);
    if (it == properties_.end())
    {
        throw ConvertError("Property not found: " + key);
    }
    return it->second;
}

bool Environment::hasProperty(const std::string& key) const {
    return properties_.find(key) != properties_.end();
}

void Environment::setProperty(const std::string& key, const std::any& value) {
    properties_[key] = value;
}