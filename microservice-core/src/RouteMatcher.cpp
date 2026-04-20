#include "adapters/primary/RouteMatcher.hpp"

/**
 * @file RouteMatcher.cpp
 * @brief RouteMatcher implementation
 * @author Anton Tobolkin
 */

bool RouteMatcher::matches(const std::string& pattern, const std::string& path)
{
    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() == '/')
    {
        normalizedPath.pop_back();
    }
    
    auto patternSegments = split(pattern, '/');
    auto pathSegments = split(normalizedPath, '/');
    
    if (patternSegments.size() != pathSegments.size())
    {
        return false;
    }
    
    for (size_t i = 0; i < patternSegments.size(); ++i)
    {
        if (patternSegments[i] == "*")
        {
            continue;
        }
        
        if (patternSegments[i] != pathSegments[i])
        {
            return false;
        }
    }
    
    return true;
}

std::vector<std::string> RouteMatcher::split(const std::string& str, char delimiter)
{
    std::vector<std::string> segments;
    std::string segment;
    bool hasTrailingDelimiter = false;
    
    for (char ch : str)
    {
        if (ch == delimiter)
        {
            if (!segment.empty())
            {
                segments.push_back(segment);
                segment.clear();
            }
            hasTrailingDelimiter = true;
        }
        else
        {
            segment += ch;
            hasTrailingDelimiter = false;
        }
    }
    
    if (!segment.empty())
    {
        segments.push_back(segment);
    }
    
    return segments;
}
