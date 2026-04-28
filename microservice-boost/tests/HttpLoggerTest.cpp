#include <gtest/gtest.h>
#include "adapters/secondary/HttpLogger.hpp"
#include "settings/HttpLogSettings.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "adapters/secondary/TestLogger.hpp"
#include "ports/output/IShutdown.hpp"

/**
 * @file HttpLoggerTest.cpp
 * @brief HttpLogger tests
 * @author Anton Tobolkin
 */

class MockHttpClient : public IHttpClient
{
public:
    std::vector<std::string> requests;
    std::map<std::string, std::string> lastHeaders;
    bool shouldFail = false;

    HttpClientResult send(const IRequest& request, IResponse& response) override
    {
        requests.push_back(request.getBody());
        lastHeaders = request.getHeaders();
        if (shouldFail) {
            return {HttpClientError::UnknownError, "mock error"};
        }
        response.setStatus(200);
        return {HttpClientError::None, ""};
    }
};

TEST(HttpLoggerTest, DoesNotCrashOnLog)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST");
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, settings, fallback);
    logger.log(LogLevel::Info, "App", "started");
    logger.log(LogLevel::Error, "App", "error occurred");
    logger.flush();
    logger.stop();
}

TEST(HttpLoggerTest, FallbackLoggerIsUsedWhenUrlNotConfigured)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST");
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Warn, "Test", "test message");
    logger.flush();
    logger.stop();

    bool found = false;
    for (const auto& entry : fallback->getEntries()) {
        if (entry.category == "HttpLogger" &&
            entry.message.find("URL not configured") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST(HttpLoggerTest, FlushSendsBatchToHttpClient)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST", "http://localhost:9999");
    auto fallback = std::make_shared<NullLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Info, "App", "message1");
    logger.log(LogLevel::Info, "App", "message2");
    logger.flush();
    logger.stop();

    EXPECT_EQ(mockClient->requests.size(), 1u);
}

TEST(HttpLoggerTest, CanBeDestroyedWithoutFlush)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    auto logger = std::make_shared<HttpLogger>(mockClient, settings, fallback);
    logger->log(LogLevel::Info, "App", "message");
}

TEST(HttpLoggerTest, DefaultFallbackIsNullLogger)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST");
    HttpLogger logger(mockClient, settings, nullptr);
    logger.log(LogLevel::Info, "App", "test");
    logger.stop();
}

TEST(HttpLoggerTest, LogEntriesContainCorrectData)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST", "http://localhost:9999");
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Info, "MyCategory", "MyMessage");
    logger.flush();
    logger.stop();

    ASSERT_EQ(mockClient->requests.size(), 1u);
    EXPECT_TRUE(mockClient->requests[0].find("\"category\":\"MyCategory\"") != std::string::npos);
    EXPECT_TRUE(mockClient->requests[0].find("\"message\":\"MyMessage\"") != std::string::npos);
}

TEST(HttpLoggerTest, FailedHttpCallUsesFallback)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    mockClient->shouldFail = true;
    auto settings = std::make_shared<HttpLogSettings>("TEST", "http://localhost:9999");
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Error, "App", "test");
    logger.flush();
    logger.stop();

    bool found = false;
    for (const auto& entry : fallback->getEntries()) {
        if (entry.category == "HttpLogger" &&
            entry.message.find("Failed to send logs") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST(HttpLoggerTest, ImplementsIShutdown)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    auto shutdownPtr = dynamic_cast<IShutdown*>(&logger);
    ASSERT_NE(shutdownPtr, nullptr);
    EXPECT_EQ(shutdownPtr->name(), "HttpLogger");

    logger.log(LogLevel::Info, "App", "test");
    logger.shutdown();
}

TEST(HttpLoggerTest, ShutdownFlushesPendingLogs)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<HttpLogSettings>("TEST", "http://localhost:9999");
    auto fallback = std::make_shared<NullLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Info, "App", "message1");
    logger.log(LogLevel::Info, "App", "message2");

    EXPECT_TRUE(mockClient->requests.empty());

    logger.shutdown(std::chrono::milliseconds(1000));

    EXPECT_EQ(mockClient->requests.size(), 1u);
}

TEST(HttpLoggerTest, ShutdownRespectsTimeout)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    mockClient->shouldFail = true;
    auto settings = std::make_shared<HttpLogSettings>("TEST", "http://localhost:9999");
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Info, "App", "message1");
    logger.log(LogLevel::Info, "App", "message2");
    logger.log(LogLevel::Info, "App", "message3");

    logger.shutdown(std::chrono::milliseconds(1));

    EXPECT_FALSE(mockClient->requests.empty());
}