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

private:
    /**
     * @brief Split string into segments by delimiter
     */
    static std::vector<std::string> split(const std::string& str, char delimiter);
};
