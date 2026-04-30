#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "domain/error/BadRequestError.hpp"
#include <nlohmann/json.hpp>

/**
 * @file JsonValidator.hpp
 * @brief JSON validation handler
 * @author Anton Tobolkin
 */

/**
 * @class JsonValidator
 * @brief Validates that request body is valid JSON
 */
class JsonValidator : public IHttpHandler
{
public:
    /**
     * @brief Validate JSON request
     * @param req HTTP request
     * @param res HTTP response (unused)
     */
    void handle(IRequest &req, IResponse & /*res*/) override;
};