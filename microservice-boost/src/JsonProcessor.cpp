#include "adapters/primary/JsonProcessor.hpp"
#include "domain/error/BadRequestError.hpp"
#include <memory>

JsonProcessor::JsonProcessor(std::shared_ptr<IJsonToEnvConverter> converter)
    : converter_(std::move(converter))
{
}

void JsonProcessor::handle(IRequest& req, IResponse& /*res*/) {
    if (!req.isJson())
    {
        throw BadRequestError("Content-Type must be application/json");
    }

    auto env = converter_->convert(req.getBody());
    req.setObject(JSON_OBJECT_KEY, env);
}

std::string JsonProcessor::name() const {
    return "JsonProcessor";
}