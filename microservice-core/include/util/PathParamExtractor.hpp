#pragma once

#include <string>
#include <optional>
#include <cstddef>

class PathParamExtractor
{
public:
    static std::optional<std::string> getByIndex(
        const std::string &path,
        const std::string &pathPattern,
        size_t index);
};
