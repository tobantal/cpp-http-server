#pragma once

#include "ports/output/ILogger.hpp"

/**
 * @file NullLogger.hpp
 * @brief Null logger implementation (no-op)
 * @author Anton Tobolkin
 */

/**
 * @class NullLogger
 * @brief Logger that discards all messages
 */
class NullLogger : public ILogger
{
public:
    /**
     * @brief Log a message (no-op)
     * @param level Log level
     * @param category Log category
     * @param message Log message
     */
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;
};
