#include <gtest/gtest.h>
#include "adapters/secondary/HttpLogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "adapters/secondary/TestLogger.hpp"

/**
 * @file HttpLoggerTest.cpp
 * @brief HttpLogger tests
 * @author Anton Tobolkin
 */

class MockHttpClient : public IHttpClient
{
public:
    std::vector<std::string> requests;
    bool shouldFail = false;

    HttpClientResult send(const IRequest& request, IResponse& response) override
    {
        requests.push_back(request.getBody());
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
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, fallback);
    logger.log(LogLevel::Info, "App", "started");
    logger.log(LogLevel::Error, "App", "error occurred");
    logger.stop();
}

TEST(HttpLoggerTest, FallbackLoggerIsUsedWhenUrlNotConfigured)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, fallback);
    logger.log(LogLevel::Warn, "Test", "test message");
    logger.flush();
    logger.stop();

    bool found = false;
    for (const auto& entry : fallback->getEntries()) {
        if (entry.category == "HttpLogger" &&
            entry.message.find("HTTP_URL not configured") != std::string::npos) {
            found = true;
            break;
        }
    }
    EXPECT_TRUE(found);
}

TEST(HttpLoggerTest, FlushSendsBatchToHttpClient)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto fallback = std::make_shared<NullLogger>();
    HttpLogger logger(mockClient, fallback);

    setenv("HTTP_URL", "http://localhost:9999", 1);
    logger.log(LogLevel::Info, "App", "message1");
    logger.log(LogLevel::Info, "App", "message2");
    logger.flush();
    logger.stop();
    unsetenv("HTTP_URL");

    EXPECT_EQ(mockClient->requests.size(), 1u);
}

TEST(HttpLoggerTest, CanBeDestroyedWithoutFlush)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto fallback = std::make_shared<NullLogger>();
    auto logger = std::make_shared<HttpLogger>(mockClient, fallback);
    logger->log(LogLevel::Info, "App", "message");
}

TEST(HttpLoggerTest, DefaultFallbackIsNullLogger)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    HttpLogger logger(mockClient, nullptr);
    logger.log(LogLevel::Info, "App", "test");
    logger.stop();
}

TEST(HttpLoggerTest, LogEntriesContainCorrectData)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, fallback);

    setenv("HTTP_URL", "http://localhost:9999", 1);
    logger.log(LogLevel::Info, "MyCategory", "MyMessage");
    logger.flush();
    logger.stop();
    unsetenv("HTTP_URL");

    ASSERT_EQ(mockClient->requests.size(), 1u);
    EXPECT_TRUE(mockClient->requests[0].find("\"category\":\"MyCategory\"") != std::string::npos);
    EXPECT_TRUE(mockClient->requests[0].find("\"message\":\"MyMessage\"") != std::string::npos);
}

TEST(HttpLoggerTest, FailedHttpCallUsesFallback)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    mockClient->shouldFail = true;
    auto fallback = std::make_shared<TestLogger>();
    HttpLogger logger(mockClient, fallback);

    setenv("HTTP_URL", "http://localhost:9999", 1);
    logger.log(LogLevel::Error, "App", "test");
    logger.flush();
    logger.stop();
    unsetenv("HTTP_URL");

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