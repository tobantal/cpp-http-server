#pragma once

#include "ports/input/IJsonProcessor.hpp"
#include "domain/error/BadRequestError.hpp"
#include <nlohmann/json.hpp>
#include <memory>

/**
 * @file JsonProcessor.hpp
 * @brief JSON processing handler implementation
 * @author AI-Coder
 */

/**
 * @class JsonProcessor
 * @brief Parses JSON request body and stores result in request
 *
 * Validates Content-Type, parses JSON string once, and stores
 * JsonObject in request attributes for downstream handlers.
 */
class JsonProcessor : public IJsonProcessor
{
public:
    /**
     * @brief Process JSON request
     * @param req HTTP request
     * @param res HTTP response (unused)
     * @throws BadRequestError if Content-Type is not JSON or JSON is invalid
     */
    void handle(IRequest& req, IResponse& /*res*/) override
    {
        if (!req.isJson())
        {
            throw BadRequestError("Content-Type must be application/json");
        }

        try
        {
            auto jsonObj = std::make_shared<JsonObject>(req.getBody());
            req.setObject(JSON_OBJECT_KEY, jsonObj);
        }
        catch (const nlohmann::json::parse_error& e)
        {
            throw BadRequestError(std::string("Invalid JSON: ") + e.what());
        }
    }

    std::string name() const override
    {
        return "JsonProcessor";
    }
};