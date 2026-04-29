/**
 * @file JsonValidator.cpp
 * @brief JSON validation handler implementation
 * @author Anton Tobolkin
 */

#include "adapters/primary/handler/JsonValidator.hpp"

void JsonValidator::handle(IRequest &req, IResponse & /*res*/)
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
