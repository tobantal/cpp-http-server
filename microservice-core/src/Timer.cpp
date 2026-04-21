#include "util/Timer.hpp"

/**
 * @file Timer.cpp
 * @brief Timer utility implementation
 * @author Anton Tobolkin
 */

void Timer::start()
{
    elapsed_ = std::chrono::nanoseconds(0);
    start_ = std::chrono::steady_clock::now();
    running_ = true;
}

void Timer::stop()
{
    if (running_)
    {
        elapsed_ = std::chrono::steady_clock::now() - start_;
        running_ = false;
    }
}

int64_t Timer::elapsed(TimeUnit unit) const
{
    std::chrono::nanoseconds dur = running_
                                       ? std::chrono::steady_clock::now() - start_
                                       : elapsed_;

    switch (unit)
    {
    case TimeUnit::Nanos:
        return std::chrono::duration_cast<std::chrono::nanoseconds>(dur).count();
    case TimeUnit::Micros:
        return std::chrono::duration_cast<std::chrono::microseconds>(dur).count();
    case TimeUnit::Millis:
        return std::chrono::duration_cast<std::chrono::milliseconds>(dur).count();
    case TimeUnit::Seconds:
        return std::chrono::duration_cast<std::chrono::seconds>(dur).count();
    }
    return 0;
}

std::string Timer::show(TimeUnit unit) const
{
    int64_t value = elapsed(unit);
    std::string result = std::to_string(value);
    switch (unit)
    {
    case TimeUnit::Nanos:
        result.append("ns");
        break;
    case TimeUnit::Micros:
        result.append("us");
        break;
    case TimeUnit::Millis:
        result.append("ms");
        break;
    case TimeUnit::Seconds:
        result.append("s");
        break;
    }
    return result;
}