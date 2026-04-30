#pragma once

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

class StringUtils
{
public:
    static std::string toLower(const std::string &str);

    static std::vector<std::string> splitPath(const std::string &path);

    static std::string escapeJson(const std::string &s);

    static std::string urlDecode(const std::string &s);
};