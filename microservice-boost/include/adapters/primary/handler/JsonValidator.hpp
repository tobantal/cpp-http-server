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
    void handle(IRequest &req, IResponse & /*res*/) override
    {
        if (!req.isJson())
        {
            throw BadRequestError(std::string("Content-Type must be application/json"));
        }

        try
        {
            auto parsed = nlohmann::json::parse(req.getBody());
            (void)parsed;
        }
        catch (const nlohmann::json::parse_error &e)
        {
            throw BadRequestError(std::string("Invalid JSON: ") + e.what());
        }
    }
};
