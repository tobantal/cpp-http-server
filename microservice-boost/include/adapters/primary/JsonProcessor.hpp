#pragma once

#include "ports/input/IJsonProcessor.hpp"
#include "ports/input/IJsonToEnvConverter.hpp"
#include "domain/error/BadRequestError.hpp"
#include <memory>

/**
 * @file JsonProcessor.hpp
 * @brief JSON processing handler implementation
 * @author Anton Tobolkin
 */

/**
 * @class JsonProcessor
 * @brief Parses JSON request body and stores result in request
 *
 * Validates Content-Type, parses JSON string once using converter,
 * and stores IEnvironment in request attributes for downstream handlers.
 */
class JsonProcessor : public IJsonProcessor
{
public:
    /**
     * @brief Construct JsonProcessor with JSON to Environment converter
     * @param converter Converter for JSON string to IEnvironment
     */
    explicit JsonProcessor(std::shared_ptr<IJsonToEnvConverter> converter);

    /**
     * @brief Process JSON request
     * @param req HTTP request
     * @param res HTTP response (unused)
     * @throws BadRequestError if Content-Type is not JSON
     * @throws ConvertError if JSON parsing fails
     */
    void handle(IRequest& req, IResponse& /*res*/) override;

    std::string name() const override;

private:
    std::shared_ptr<IJsonToEnvConverter> converter_;
};