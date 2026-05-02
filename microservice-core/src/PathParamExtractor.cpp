#include "util/PathParamExtractor.hpp"
#include "util/StringUtils.hpp"
#include "adapters/primary/RouteMatcher.hpp"
#include <optional>
#include <string>
#include <vector>

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
        if (RouteMatcher::isWildcard(patternSegments[i]))
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

std::optional<std::string> PathParamExtractor::getByName(
    const std::string &path,
    const std::string &pathPattern,
    const std::string &paramName)
{
    if (pathPattern.empty() || paramName.empty())
    {
        return std::nullopt;
    }

    auto pathSegments = StringUtils::splitPath(path);
    auto patternSegments = StringUtils::splitPath(pathPattern);

    for (size_t i = 0; i < patternSegments.size() && i < pathSegments.size(); ++i)
    {
        if (RouteMatcher::isNamedParam(patternSegments[i]))
        {
            std::string name = RouteMatcher::paramName(patternSegments[i]);
            if (name == paramName)
            {
                return pathSegments[i];
            }
        }
    }

    return std::nullopt;
}