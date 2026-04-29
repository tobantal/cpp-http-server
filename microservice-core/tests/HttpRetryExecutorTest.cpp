#include <gtest/gtest.h>
#include "application/HttpRetryExecutor.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <atomic>

class MockHttpRetrySettings : public IHttpRetrySettings
{
public:
    int getMaxAttempts() const override { return maxAttempts_; }
    std::chrono::milliseconds getBaseDelay() const override { return baseDelay_; }
    double getMultiplier() const override { return multiplier_; }
    std::chrono::milliseconds getMaxDelay() const override { return maxDelay_; }
    const std::set<int>& getRetryableStatuses() const override { return retryableStatuses_; }
    bool isRetryOnNetworkErrorEnabled() const override { return retryOnNetworkError_; }

    int maxAttempts_ = 3;
    std::chrono::milliseconds baseDelay_{100};
    double multiplier_ = 2.0;
    std::chrono::milliseconds maxDelay_{10000};
    std::set<int> retryableStatuses_ = {500, 502, 503, 504};
    bool retryOnNetworkError_ = true;
};

class HttpRetryExecutorTest : public ::testing::Test
{
protected:
    MockHttpRetrySettings settings;
    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>();
    HttpRetryExecutor executor{std::make_shared<MockHttpRetrySettings>(settings), logger};
};

TEST_F(HttpRetryExecutorTest, SuccessOnFirstAttempt)
{
    int callCount = 0;
    auto result = executor.execute([&]() {
        ++callCount;
        return HttpClientResult{HttpClientError::None, ""};
    });

    EXPECT_TRUE(result.ok());
    EXPECT_EQ(callCount, 1);
}

TEST_F(HttpRetryExecutorTest, RetriesOnNetworkError)
{
    int callCount = 0;
    auto result = executor.execute([&]() {
        ++callCount;
        if (callCount < 3) {
            return HttpClientResult{HttpClientError::ConnectTimeout, ""};
        }
        return HttpClientResult{HttpClientError::None, ""};
    });

    EXPECT_TRUE(result.ok());
    EXPECT_EQ(callCount, 3);
}

TEST_F(HttpRetryExecutorTest, ReturnsFailureAfterMaxAttemptsOnNetworkError)
{
    int callCount = 0;
    auto result = executor.execute([&]() {
        ++callCount;
        return HttpClientResult{HttpClientError::ConnectTimeout, ""};
    });

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(callCount, 3);
}

TEST_F(HttpRetryExecutorTest, NoRetryOnDnsFailed)
{
    int callCount = 0;
    auto result = executor.execute([&]() {
        ++callCount;
        return HttpClientResult{HttpClientError::DnsFailed, ""};
    });

    EXPECT_FALSE(result.ok());
    EXPECT_EQ(callCount, 1);
}

TEST_F(HttpRetryExecutorTest, ShouldRetryOnHttpStatus_5xx_ReturnsTrue)
{
    EXPECT_TRUE(executor.shouldRetryOnHttpStatus(500));
    EXPECT_TRUE(executor.shouldRetryOnHttpStatus(502));
    EXPECT_TRUE(executor.shouldRetryOnHttpStatus(503));
    EXPECT_TRUE(executor.shouldRetryOnHttpStatus(504));
}

TEST_F(HttpRetryExecutorTest, ShouldRetryOnHttpStatus_4xx_ReturnsFalse)
{
    EXPECT_FALSE(executor.shouldRetryOnHttpStatus(400));
    EXPECT_FALSE(executor.shouldRetryOnHttpStatus(404));
    EXPECT_FALSE(executor.shouldRetryOnHttpStatus(401));
}