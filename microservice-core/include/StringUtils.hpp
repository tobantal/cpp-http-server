#pragma once

#include <algorithm>
#include <string>
#include <vector>

class StringUtils
{
public:
    static std::string toLower(const std::string &str)
    {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    static std::vector<std::string> splitPath(const std::string &path)
    {
        std::vector<std::string> segments;
        std::string segment;

        for (char ch : path)
        {
            if (ch == '/')
            {
                if (!segment.empty())
                {
                    segments.push_back(segment);
                    segment.clear();
                }
            }
            else
            {
                segment += ch;
            }
        }

        if (!segment.empty())
        {
            segments.push_back(segment);
        }

        return segments;
    }
};