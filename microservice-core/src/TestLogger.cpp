#include "TestLogger.hpp"

void TestLogger::log(LogLevel level, std::string_view category, std::string_view message)
{
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.push_back({level, std::string(category), std::string(message)});
}

std::vector<LogEntry> TestLogger::getEntries() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_;
}

void TestLogger::clear()
{
    std::lock_guard<std::mutex> lock(mutex_);
    entries_.clear();
}

size_t TestLogger::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.size();
}

const LogEntry& TestLogger::at(size_t index) const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return entries_.at(index);
}