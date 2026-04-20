#pragma once

#include "ports/output/ILogger.hpp"
#include <string>
#include <vector>
#include <mutex>

/**
 * @file TestLogger.hpp
 * @brief Test logger implementation for unit tests
 * @author Anton Tobolkin
 */

/**
 * @struct LogEntry
 * @brief Single log entry
 */
struct LogEntry
{
    LogLevel level;
    std::string category;
    std::string message;
};

/**
 * @class TestLogger
 * @brief Logger that stores log entries for verification in tests
 */
class TestLogger : public ILogger
{
public:
    /**
     * @brief Log a message
     * @param level Log level
     * @param category Log category
     * @param message Log message
     */
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;

    /**
     * @brief Get all log entries
     * @return Vector of log entries
     */
    std::vector<LogEntry> getEntries() const;

    /**
     * @brief Clear all log entries
     */
    void clear();

    /**
     * @brief Get number of log entries
     * @return Number of entries
     */
    size_t size() const;

    /**
     * @brief Get log entry at index
     * @param index Entry index
     * @return Const reference to log entry
     */
    const LogEntry& at(size_t index) const;

private:
    mutable std::mutex mutex_;
    std::vector<LogEntry> entries_;
};
