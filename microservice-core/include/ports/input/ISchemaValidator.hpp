#pragma once

#include "ports/input/IHttpHandler.hpp"

/**
 * @file ISchemaValidator.hpp
 * @brief Interface for schema validation handler
 * @author Anton Tobolkin
 */

/**
 * @class ISchemaValidator
 * @brief Validates request fields against a Schema
 *
 * Place in ChainHandler after JsonProcessor and before
 * the business handler. On validation failure throws
 * BadRequestError(400) — chain stops with HTTP 400.
 */
class ISchemaValidator : public IHttpHandler
{
public:
    ~ISchemaValidator() override = default;
};