#pragma once

#include "ports/input/IHttpHandler.hpp"

class IJsonProcessor : public IHttpHandler
{
public:
    static constexpr const char* JSON_OBJECT_KEY = "json_object";

    ~IJsonProcessor() override = default;
};