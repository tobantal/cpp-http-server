#pragma once

#include "ports/output/IEnvironment.hpp"
#include <nlohmann/json.hpp>
#include <string>
#include <map>

class JsonObject : public IEnvironment
{
public:
    explicit JsonObject(const std::string& body);

    std::string toJson();

    std::any getProperty(const std::string& key) const override;
    void setProperty(const std::string& key, const std::any& value) override;

private:
    nlohmann::json json_;
    std::map<std::string, std::any> properties_;
};