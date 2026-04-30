#pragma once

#include <optional>
#include <string>
#include <vector>

class PathParamExtractor
{
public:
    static std::optional<std::string> getByIndex(
        const std::string &path,
        const std::string &pathPattern,
        size_t index);
};