#include "RouteMatcher.hpp"

/**
 * @file RouteMatcher.cpp
 * @brief Реализация утилиты сопоставления путей
 * @author Anton Tobolkin
 */

bool RouteMatcher::matches(const std::string& pattern, const std::string& path)
{
    // Нормализуем trailing slash только для path (для wildcard matching)
    // но сохраняем информацию для точного сравнения
    std::string normalizedPath = path;
    if (!normalizedPath.empty() && normalizedPath.back() == '/')
    {
        normalizedPath.pop_back();
    }
    
    auto patternSegments = split(pattern, '/');
    auto pathSegments = split(normalizedPath, '/');
    
    // Количество сегментов должно совпадать
    if (patternSegments.size() != pathSegments.size())
    {
        return false;
    }
    
    // Сравниваем каждый сегмент
    for (size_t i = 0; i < patternSegments.size(); ++i)
    {
        // * соответствует любому сегменту
        if (patternSegments[i] == "*")
        {
            continue;
        }
        
        // Остальные должны совпадать точно
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
    
    // Добавляем последний сегмент, если он не пустой
    if (!segment.empty())
    {
        segments.push_back(segment);
    }
    
    // Если строка заканчивалась на delimiter, добавляем пустой сегмент
    if (hasTrailingDelimiter && !str.empty())
    {
        segments.push_back("");
    }
    
    return segments;
}