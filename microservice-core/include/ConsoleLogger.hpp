#pragma once

#include "ILogger.hpp"

class ConsoleLogger : public ILogger
{
public:
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;
};