#pragma once

#include <string_view>
#include <cstdint>

enum class LogLevel : uint8_t
{
    Debug,
    Info,
    Warn,
    Error
};

std::string_view logLevelToString(LogLevel level) noexcept;

class ILogger
{
public:
    virtual ~ILogger() = default;

    virtual void log(LogLevel level,
                     std::string_view category,
                     std::string_view message) = 0;
};