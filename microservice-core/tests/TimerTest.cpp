#include <gtest/gtest.h>
#include "util/Timer.hpp"
#include <thread>
#include <chrono>

/**
 * @file TimerTest.cpp
 * @brief Unit tests for Timer utility
 */

TEST(TimerTest, StartStopElapsedMillis)
{
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.stop();

    int64_t elapsed = timer.elapsed(TimeUnit::Millis);
    EXPECT_GE(elapsed, 40);
    EXPECT_LT(elapsed, 500);
}

TEST(TimerTest, ShowDefaultUnit)
{
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    timer.stop();

    std::string result = timer.show();
    EXPECT_NE(result.find("ms"), std::string::npos);
}

TEST(TimerTest, ShowNanos)
{
    Timer timer;
    timer.start();
    timer.stop();

    std::string result = timer.show(TimeUnit::Nanos);
    EXPECT_NE(result.find("ns"), std::string::npos);
}

TEST(TimerTest, ShowMicros)
{
    Timer timer;
    timer.start();
    timer.stop();

    std::string result = timer.show(TimeUnit::Micros);
    EXPECT_NE(result.find("us"), std::string::npos);
}

TEST(TimerTest, ShowSeconds)
{
    Timer timer;
    timer.start();
    timer.stop();

    std::string result = timer.show(TimeUnit::Seconds);
    EXPECT_NE(result.find("s"), std::string::npos);
}

TEST(TimerTest, ElapsedWithoutStartReturnsZero)
{
    Timer timer;
    EXPECT_EQ(timer.elapsed(TimeUnit::Millis), 0);
}

TEST(TimerTest, ElapsedWhileRunning)
{
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    int64_t elapsed = timer.elapsed(TimeUnit::Millis);
    EXPECT_GE(elapsed, 5);

    timer.stop();
    int64_t elapsedAfterStop = timer.elapsed(TimeUnit::Millis);
    EXPECT_GE(elapsedAfterStop, 5);
}

TEST(TimerTest, StartResetsElapsed)
{
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    timer.stop();

    int64_t first = timer.elapsed(TimeUnit::Millis);
    EXPECT_GE(first, 20);

    timer.start();
    timer.stop();

    int64_t second = timer.elapsed(TimeUnit::Millis);
    EXPECT_LT(second, first);
}

TEST(TimerTest, TimeUnitConversion)
{
    Timer timer;
    timer.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    timer.stop();

    int64_t millis = timer.elapsed(TimeUnit::Millis);
    int64_t micros = timer.elapsed(TimeUnit::Micros);

    EXPECT_GE(millis, 40);
    EXPECT_GE(micros, 40000);
    EXPECT_NEAR(micros, millis * 1000, 5000);
}