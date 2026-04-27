#pragma once

#include "ports/input/IJsonProcessor.hpp"
#include "domain/error/BadRequestError.hpp"
#include <nlohmann/json.hpp>
#include <memory>

class JsonProcessor : public IJsonProcessor
{
public:
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