#pragma once

#include <string_view>
#include <cstdint>

/**
 * @file ILogger.hpp
 * @brief Interface for logging
 * @author Anton Tobolkin
 */

/**
 * @enum LogLevel
 * @brief Log severity levels
 */
enum class LogLevel : uint8_t
{
    Debug,
    Info,
    Warn,
    Error
};

/**
 * @brief Convert LogLevel to string representation
 * @param level Log level
 * @return String representation of the log level
 */
std::string_view logLevelToString(LogLevel level) noexcept;

/**
 * @class ILogger
 * @brief Interface for logging
 */
class ILogger
{
public:
    virtual ~ILogger() = default;

    /**
     * @brief Log a message
     * @param level Log severity level
     * @param category Log category
     * @param message Log message
     */
    virtual void log(LogLevel level,
                     std::string_view category,
                     std::string_view message) = 0;
};
