#pragma once

#include "JsonParseError.hpp"
#include <nlohmann/json.hpp>
#include <string>

template <typename T>
std::string serialize(const T &obj)
{
    nlohmann::json j = obj;
    return j.dump();
}

template <typename T>
T deserialize(const std::string &body)
{
    try
    {
        nlohmann::json j = nlohmann::json::parse(body);
        return j.get<T>();
    }
    catch (const nlohmann::json::parse_error &e)
    {
        throw JsonParseError(std::string("JSON parse error: ") + e.what());
    }
    catch (const nlohmann::json::type_error &e)
    {
        throw JsonParseError(std::string("JSON type error: ") + e.what());
    }
    catch (const nlohmann::json::out_of_range &e)
    {
        throw JsonParseError(std::string("JSON out of range: ") + e.what());
    }
    catch (const nlohmann::json::exception &e)
    {
        throw JsonParseError(std::string("JSON error: ") + e.what());
    }
}