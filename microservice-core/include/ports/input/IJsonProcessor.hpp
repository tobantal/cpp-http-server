#pragma once

#include "ports/input/IHttpHandler.hpp"

/**
 * @file IJsonProcessor.hpp
 * @brief Interface for JSON processing handler
 * @author AI-Coder
 */

/**
 * @class IJsonProcessor
 * @brief Interface for processing JSON request body
 *
 * Implementations parse JSON string once and store result
 * in request via setObject() for subsequent handlers.
 */
class IJsonProcessor : public IHttpHandler
{
public:
    /**
     * @brief Key for storing JsonObject in request
     */
    static constexpr const char* JSON_OBJECT_KEY = "json_object";

    ~IJsonProcessor() override = default;
};