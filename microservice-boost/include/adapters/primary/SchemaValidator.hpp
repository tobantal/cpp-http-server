#pragma once

#include "ports/input/ISchemaValidator.hpp"
#include "domain/schema/Schema.hpp"
#include <memory>

/**
 * @file SchemaValidator.hpp
 * @brief Schema validation handler implementation
 * @author Anton Tobolkin
 */

/**
 * @class SchemaValidator
 * @brief Validates IEnvironment fields against a Schema
 *
 * Place in ChainHandler after JsonProcessor. Retrieves
 * IEnvironment from request and validates each field.
 * On failure throws BadRequestError(400) with all violations.
 */
class SchemaValidator : public ISchemaValidator
{
public:
    /**
     * @brief Construct SchemaValidator with schema
     * @param schema Schema describing expected fields
     */
    explicit SchemaValidator(std::shared_ptr<Schema> schema);

    /**
     * @brief Validate request fields against schema
     * @param req HTTP request (must have IEnvironment stored by JsonProcessor)
     * @param res HTTP response (unused)
     * @throws BadRequestError if validation fails
     */
    void handle(IRequest& req, IResponse& /*res*/) override;

    std::string name() const override;

private:
    std::shared_ptr<Schema> schema_;
};