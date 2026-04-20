#include "ports/output/ILogger.hpp"

/**
 * @file LogLevel.cpp
 * @brief LogLevel utility implementation
 * @author Anton Tobolkin
 */

std::string_view logLevelToString(LogLevel level) noexcept
{
    switch (level)
    {
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info: return "INFO";
    case LogLevel::Warn: return "WARN";
    case LogLevel::Error: return "ERROR";
    default: return "UNKNOWN";
    }
}
