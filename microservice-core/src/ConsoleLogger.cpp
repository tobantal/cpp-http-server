#include "ConsoleLogger.hpp"
#include <iostream>

void ConsoleLogger::log(LogLevel level, std::string_view category, std::string_view message)
{
    std::cout << "[" << logLevelToString(level) << "] "
              << "[" << category << "] "
              << message << std::endl;
}