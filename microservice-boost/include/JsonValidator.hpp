#pragma once

#include "IHttpHandler.hpp"
#include "BadRequestError.hpp"
#include <nlohmann/json.hpp>

class JsonValidator : public IHttpHandler
{
public:
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