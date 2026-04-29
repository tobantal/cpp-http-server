/**
 * @file PathParamExtractor.cpp
 * @brief PathParamExtractor implementation
 * @author Anton Tobolkin
 */

#include "util/PathParamExtractor.hpp"

#include <string>
#include <vector>
#include <optional>
#include <cstddef>
#include "util/StringUtils.hpp"

/**
 * @brief Extract path parameter by index from wildcard pattern
 * @param path Request path (e.g., "/api/users/123")
 * @param pathPattern Pattern with wildcards (e.g., "/api/users/*")
 * @param index Zero-based index of wildcard to extract
 * @return Parameter value or std::nullopt if not found
 *
 * Example: getByIndex("/api/users/123", "/api/users/*", 0) returns "123"
 */
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
