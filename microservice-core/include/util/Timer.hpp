#pragma once

#include <chrono>
#include <string>
#include <cstdint>

/**
 * @file Timer.hpp
 * @brief Timer utility for measuring elapsed time
 * @author Anton Tobolkin
 */

/**
 * @enum TimeUnit
 * @brief Time unit for elapsed/show methods
 */
enum class TimeUnit
{
    Nanos,
    Micros,
    Millis,
    Seconds
};

/**
 * @class Timer
 * @brief Measures elapsed time between start() and stop() calls
 *
 * Usage:
 *   Timer t;
 *   t.start();
 *   // ... do work ...
 *   t.stop();
 *   logger->log(Debug, "Timer", "Elapsed: " + t.show());
 *
 * If stop() is not called, elapsed()/show() return time since start().
 */
class Timer
{
public:
    Timer() = default;

    /**
     * @brief Start (or restart) the timer
     */
    void start();

    /**
     * @brief Stop the timer and save elapsed duration
     */
    void stop();

    /**
     * @brief Get elapsed time in specified unit
     * @param unit Time unit (default: Millis)
     * @return Elapsed time as int64_t in the specified unit.
     *         If timer is running, returns time since start().
     *         If timer is stopped, returns saved elapsed duration.
     *         If timer was never started, returns 0.
     */
    int64_t elapsed(TimeUnit unit = TimeUnit::Millis) const;

    /**
     * @brief Get human-readable elapsed time string with unit suffix
     * @param unit Time unit (default: Millis)
     * @return String like "42ms", "1500ns", "3s"
     */
    std::string show(TimeUnit unit = TimeUnit::Millis) const;

private:
    /** @brief Start time point */
    std::chrono::steady_clock::time_point start_;

    /** @brief Saved elapsed duration */
    std::chrono::nanoseconds elapsed_{0};

    /** @brief Whether timer is running */
    bool running_{false};
};