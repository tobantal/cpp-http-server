#pragma once

#include "ILogger.hpp"
#include <string>
#include <vector>
#include <mutex>

struct LogEntry
{
    LogLevel level;
    std::string category;
    std::string message;
};

class TestLogger : public ILogger
{
public:
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;

    std::vector<LogEntry> getEntries() const;
    void clear();
    size_t size() const;
    const LogEntry& at(size_t index) const;

private:
    mutable std::mutex mutex_;
    std::vector<LogEntry> entries_;
};