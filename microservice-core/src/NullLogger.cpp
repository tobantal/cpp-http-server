#include "adapters/secondary/NullLogger.hpp"

/**
 * @file NullLogger.cpp
 * @brief NullLogger implementation
 * @author Anton Tobolkin
 */

void NullLogger::log(LogLevel, std::string_view, std::string_view)
{
}
