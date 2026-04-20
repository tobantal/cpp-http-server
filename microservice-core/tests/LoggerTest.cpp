#include <gtest/gtest.h>
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/ConsoleLogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "adapters/secondary/TestLogger.hpp"

TEST(LogLevelTest, EnumValues)
{
    EXPECT_EQ(static_cast<int>(LogLevel::Debug), 0);
    EXPECT_EQ(static_cast<int>(LogLevel::Info), 1);
    EXPECT_EQ(static_cast<int>(LogLevel::Warn), 2);
    EXPECT_EQ(static_cast<int>(LogLevel::Error), 3);
}

TEST(LogLevelTest, ToString)
{
    EXPECT_EQ(logLevelToString(LogLevel::Debug), "DEBUG");
    EXPECT_EQ(logLevelToString(LogLevel::Info), "INFO");
    EXPECT_EQ(logLevelToString(LogLevel::Warn), "WARN");
    EXPECT_EQ(logLevelToString(LogLevel::Error), "ERROR");
}

TEST(NullLoggerTest, DoesNotCrash)
{
    NullLogger logger;
    logger.log(LogLevel::Error, "test", "message");
    logger.log(LogLevel::Info, "App", "started");
}

TEST(TestLoggerTest, CapturesEntries)
{
    TestLogger logger;
    logger.log(LogLevel::Info, "App", "started");
    logger.log(LogLevel::Error, "Session", "timeout");

    EXPECT_EQ(logger.size(), 2u);

    EXPECT_EQ(logger.at(0).level, LogLevel::Info);
    EXPECT_EQ(logger.at(0).category, "App");
    EXPECT_EQ(logger.at(0).message, "started");

    EXPECT_EQ(logger.at(1).level, LogLevel::Error);
    EXPECT_EQ(logger.at(1).category, "Session");
    EXPECT_EQ(logger.at(1).message, "timeout");
}

TEST(TestLoggerTest, Clear)
{
    TestLogger logger;
    logger.log(LogLevel::Info, "App", "msg");
    EXPECT_EQ(logger.size(), 1u);

    logger.clear();
    EXPECT_EQ(logger.size(), 0u);
}

TEST(TestLoggerTest, GetEntriesReturnsCopy)
{
    TestLogger logger;
    logger.log(LogLevel::Info, "App", "msg");
    auto entries = logger.getEntries();
    EXPECT_EQ(entries.size(), 1u);

    logger.clear();
    EXPECT_EQ(entries.size(), 1u);
}

TEST(TestLoggerTest, CategoryAndMessageAreCopied)
{
    TestLogger logger;
    std::string cat = "App";
    std::string msg = "test";
    logger.log(LogLevel::Info, cat, msg);

    cat = "changed";
    msg = "changed";

    EXPECT_EQ(logger.at(0).category, "App");
    EXPECT_EQ(logger.at(0).message, "test");
}

TEST(ConsoleLoggerTest, DoesNotCrash)
{
    ConsoleLogger logger;
    logger.log(LogLevel::Info, "App", "started");
    logger.log(LogLevel::Error, "Session", "error");
}

TEST(ILoggerInterfaceTest, NullLoggerViaInterface)
{
    std::unique_ptr<ILogger> logger = std::make_unique<NullLogger>();
    logger->log(LogLevel::Error, "test", "msg");
}

TEST(ILoggerInterfaceTest, TestLoggerViaInterface)
{
    TestLogger concrete;
    ILogger &logger = concrete;
    logger.log(LogLevel::Warn, "App", "warning");

    EXPECT_EQ(concrete.size(), 1u);
    EXPECT_EQ(concrete.at(0).level, LogLevel::Warn);
    EXPECT_EQ(concrete.at(0).category, "App");
    EXPECT_EQ(concrete.at(0).message, "warning");
}
