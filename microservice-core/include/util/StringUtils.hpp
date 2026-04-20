#pragma once

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iomanip>
#include <random>
#include <sstream>
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

    static std::string urlDecode(const std::string &s)
    {
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

    /**
     * @brief Сгенерировать UUID v4 (thread-safe)
     * @return 32-символьная hex-строка (128 бит: timestamp XOR random + counter XOR random)
     *
     * Адаптировано из trading-platform (common::utils::UuidGenerator).
     * Формат: 32 hex-символа без дефисов (не стандартный UUID формат).
     */
    static std::string generateUuid()
    {
        thread_local std::mt19937_64 rng(std::random_device{}());
        thread_local uint64_t counter = 0;
        thread_local std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        ++counter;

        uint64_t rnd = dist(rng);
        uint64_t hi = static_cast<uint64_t>(now) ^ (rnd << 32);
        uint64_t lo = counter ^ rnd;

        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(16) << hi
           << std::setw(16) << lo;
        return ss.str();
    }
};