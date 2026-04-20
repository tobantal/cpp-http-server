#include "adapters/secondary/ConsoleLogger.hpp"
#include <iostream>

/**
 * @file ConsoleLogger.cpp
 * @brief ConsoleLogger implementation
 * @author Anton Tobolkin
 */

void ConsoleLogger::log(LogLevel level, std::string_view category, std::string_view message)
{
    std::cout << "[" << logLevelToString(level) << "] "
              << "[" << category << "] "
              << message << std::endl;
}
