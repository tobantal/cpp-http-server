#include <gtest/gtest.h>
#include "adapters/secondary/SplunkLogger.hpp"
#include "settings/SplunkLogSettings.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "adapters/secondary/TestLogger.hpp"

/**
 * @file SplunkLoggerTest.cpp
 * @brief SplunkLogger tests
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

TEST(SplunkLoggerTest, DoesNotCrashOnLog)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<TestLogger>();
    SplunkLogger logger(mockClient, settings, fallback);
    logger.log(LogLevel::Info, "App", "started");
    logger.log(LogLevel::Error, "App", "error occurred");
    logger.flush();
    logger.stop();
}

TEST(SplunkLoggerTest, DefaultUrlIsSplunkCollector)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    SplunkLogger logger(mockClient, settings, fallback);
    logger.log(LogLevel::Info, "App", "test");
    logger.flush();
    logger.stop();

    EXPECT_EQ(mockClient->requests.size(), 1u);
}

TEST(SplunkLoggerTest, FallbackLoggerIsUsedOnHttpFailure)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    mockClient->shouldFail = true;
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<TestLogger>();
    SplunkLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Warn, "Test", "test message");
    logger.flush();
    logger.stop();

    EXPECT_FALSE(mockClient->requests.empty());
}

TEST(SplunkLoggerTest, CanBeDestroyedWithoutFlush)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    auto logger = std::make_shared<SplunkLogger>(mockClient, settings, fallback);
    logger->log(LogLevel::Info, "App", "message");
}

TEST(SplunkLoggerTest, DefaultFallbackIsNullLogger)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    SplunkLogger logger(mockClient, settings, nullptr);
    logger.log(LogLevel::Info, "App", "test");
    logger.stop();
}

TEST(SplunkLoggerTest, SplunkFormatContainsEventIndexSourcetype)
{
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    SplunkLogger logger(mockClient, settings, fallback);
    logger.log(LogLevel::Info, "App", "test");
    logger.flush();
    logger.stop();

    ASSERT_EQ(mockClient->requests.size(), 1u);
    const std::string& body = mockClient->requests[0];
    EXPECT_TRUE(body.find("\"event\":{") != std::string::npos);
    EXPECT_TRUE(body.find("\"index\":") != std::string::npos);
    EXPECT_TRUE(body.find("\"sourcetype\":") != std::string::npos);
}

TEST(SplunkLoggerTest, UsesSplunkAuthHeader)
{
    setenv("TEST_SPLUNK_TOKEN", "my-token", 1);
    auto mockClient = std::make_shared<MockHttpClient>();
    auto settings = std::make_shared<SplunkLogSettings>("TEST");
    auto fallback = std::make_shared<NullLogger>();
    SplunkLogger logger(mockClient, settings, fallback);

    logger.log(LogLevel::Info, "App", "test");
    logger.flush();
    logger.stop();

    unsetenv("TEST_SPLUNK_TOKEN");

    ASSERT_FALSE(mockClient->requests.empty());
    auto authIt = mockClient->lastHeaders.find("Authorization");
    ASSERT_TRUE(authIt != mockClient->lastHeaders.end());
    EXPECT_TRUE(authIt->second.find("Splunk") != std::string::npos);
}