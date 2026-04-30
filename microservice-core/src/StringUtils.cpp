#include "util/StringUtils.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

std::string StringUtils::toLower(const std::string &str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::vector<std::string> StringUtils::splitPath(const std::string &path) {
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

std::string StringUtils::escapeJson(const std::string &s) {
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

std::string StringUtils::urlDecode(const std::string &s) {
    std::string result;
    result.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i)
    {
        if (s[i] == '%' && i + 2 < s.size())
        {
            char hex[3] = {s[i + 1], s[i + 2], '\0'};
            char *end = nullptr;
            long value = std::strtol(hex, &end, 16);
            if (end != nullptr && *end == '\0' && value >= 0 && value <= 255)
            {
                result += static_cast<char>(value);
                i += 2;
            }
            else
            {
                result += s[i];
            }
        }
        else if (s[i] == '+')
        {
            result += ' ';
        }
        else
        {
            result += s[i];
        }
    }
    return result;
}