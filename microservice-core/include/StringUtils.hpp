#pragma once

#include <algorithm>
#include <cstdio>
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

    static std::string escapeJson(const std::string &s)
    {
        std::string result;
        result.reserve(s.size());
        for (char c : s)
        {
            switch (c)
            {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                if (static_cast<unsigned char>(c) < 0x20)
                {
                    char buf[8];
                    std::snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned char>(c));
                    result += buf;
                }
                else
                {
                    result += c;
                }
                break;
            }
        }
        return result;
    }
};