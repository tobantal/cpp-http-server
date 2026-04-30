#include "adapters/secondary/Environment.hpp"
#include <map>
#include <any>
#include <string>
#include <stdexcept>

std::any Environment::getProperty(const std::string& key) const {
    auto it = properties_.find(key);
    if (it == properties_.end())
    {
        throw std::runtime_error("Property not found: " + key);
    }
    return it->second;
}

void Environment::setProperty(const std::string& key, const std::any& value) {
    properties_[key] = value;
}