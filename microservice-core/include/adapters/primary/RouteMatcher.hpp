#pragma once

#include <string>
#include <vector>

/**
 * @file RouteMatcher.hpp
 * @brief Utility for matching paths against patterns
 * @author Anton Tobolkin
 */

/**
 * @class RouteMatcher
 * @brief Checks if a path matches a pattern
 */
class RouteMatcher
{
public:
    /**
     * @brief Check if path matches pattern
     * @param pattern Pattern with wildcards (напр. /r/)
     * @param path Path to check (e.g., /r/promo)
     * @return true if path matches pattern
     */
    static bool matches(const std::string& pattern, const std::string& path);

    /**
     * @brief Check if segment is a wildcard (* or :param)
     * @param segment Pattern segment
     * @return true if segment matches any value
     */
    static bool isWildcard(const std::string& segment);

    /**
     * @brief Check if segment is a named parameter (:paramName)
     * @param segment Pattern segment
     * @return true if segment is a named parameter
     */
    static bool isNamedParam(const std::string& segment);

    /**
     * @brief Extract parameter name from :paramName segment
     * @param segment Pattern segment (must be a named param)
     * @return Parameter name without leading ':'
     */
    static std::string paramName(const std::string& segment);

private:
    /**
     * @brief Split string into segments by delimiter
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
};
