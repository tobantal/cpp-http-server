#pragma once

#include "IConverter.hpp"
#include "ports/output/IEnvironment.hpp"
#include <memory>
#include <string>

/**
 * @file IJsonToEnvConverter.hpp
 * @brief Interface for JSON string to IEnvironment conversion
 * @author Anton Tobolkin
 */

/**
 * @class IJsonToEnvConverter
 * @brief Converts JSON string to IEnvironment
 */
class IJsonToEnvConverter : public IConverter<std::string, std::shared_ptr<IEnvironment>>
{
public:
    ~IJsonToEnvConverter() override = default;
};