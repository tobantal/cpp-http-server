#pragma once

#include <optional>
#include <string>
#include <vector>
#include <map>

class PathParamExtractor
{
public:
    /**
     * @brief Get path parameter by wildcard index
     * @param path Request path
     * @param pathPattern Pattern with wildcards
     * @param index Wildcard index (starting at 0)
     * @return Parameter value or nullopt if index out of range
     */
    static std::optional<std::string> getByIndex(
        const std::string &path,
        const std::string &pathPattern,
        size_t index);

    /**
     * @brief Get path parameter by name
     * @param path Request path
     * @param pathPattern Pattern with :paramName wildcards
     * @param paramName Parameter name (without leading ':')
     * @return Parameter value or nullopt if not found
     */
    static std::optional<std::string> getByName(
        const std::string &path,
        const std::string &pathPattern,
        const std::string &paramName);
};