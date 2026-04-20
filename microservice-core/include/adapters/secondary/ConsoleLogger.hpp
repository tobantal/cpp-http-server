#pragma once

#include "ports/output/ILogger.hpp"

/**
 * @file ConsoleLogger.hpp
 * @brief Console logger implementation
 * @author Anton Tobolkin
 */

/**
 * @class ConsoleLogger
 * @brief Logger that writes to stdout
 */
class ConsoleLogger : public ILogger
{
public:
    /**
     * @brief Log a message to console
     * @param level Log level
     * @param category Log category
     * @param message Log message
     */
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;
};
