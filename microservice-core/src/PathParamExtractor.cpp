#include "util/PathParamExtractor.hpp"

#include <string>
#include <vector>
#include <optional>
#include <cstddef>
#include "util/StringUtils.hpp"

std::optional<std::string> PathParamExtractor::getByIndex(
    const std::string &path,
    const std::string &pathPattern,
    size_t index)
{
    if (pathPattern.empty())
    {
        return std::nullopt;
    }

    auto pathSegments = StringUtils::splitPath(path);
    auto patternSegments = StringUtils::splitPath(pathPattern);

    size_t wildcardIndex = 0;
    for (size_t i = 0; i < patternSegments.size() && i < pathSegments.size(); ++i)
    {
        if (patternSegments[i] == "*")
        {
            if (wildcardIndex == index)
            {
                return pathSegments[i];
            }
            ++wildcardIndex;
        }
    }

    return std::nullopt;
}
