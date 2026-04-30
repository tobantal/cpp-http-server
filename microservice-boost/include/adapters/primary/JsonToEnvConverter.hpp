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
    std::shared_ptr<IEnvironment> convert(const std::string& input) const override;

private:
    std::any toAny(const nlohmann::json& value) const;
};