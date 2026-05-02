#include <gtest/gtest.h>
#include "domain/CircuitBreaker.hpp"
#include "application/CircuitBreakerSettings.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <thread>
#include <chrono>

class MockCircuitBreakerSettings : public ICircuitBreakerSettings
{
public:
    int failureThreshold_ = 3;
    std::chrono::milliseconds resetTimeout_{200};
    int halfOpenMaxCalls_ = 2;

    int getFailureThreshold() const override { return failureThreshold_; }
    std::chrono::milliseconds getResetTimeout() const override { return resetTimeout_; }
    int getHalfOpenMaxCalls() const override { return halfOpenMaxCalls_; }
};

class CircuitBreakerTest : public ::testing::Test
{
protected:
    MockCircuitBreakerSettings settings;
    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>();
    std::shared_ptr<CircuitBreaker> cb;

    void SetUp() override
    {
        cb = std::make_shared<CircuitBreaker>(
            std::make_shared<MockCircuitBreakerSettings>(settings), logger);
    }
};

TEST_F(CircuitBreakerTest, StartsInClosedState)
{
    EXPECT_EQ(cb->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakerTest, AllowsCallInClosedState)
{
    EXPECT_TRUE(cb->allowsCall());
}

TEST_F(CircuitBreakerTest, StaysClosedBelowThreshold)
{
    cb->recordFailure();
    cb->recordFailure();
    EXPECT_EQ(cb->state(), CircuitState::Closed);
    EXPECT_TRUE(cb->allowsCall());
}

TEST_F(CircuitBreakerTest, OpensAfterFailureThreshold)
{
    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        cb->recordFailure();
    }
    EXPECT_EQ(cb->state(), CircuitState::Open);
}

TEST_F(CircuitBreakerTest, RejectsCallInOpenState)
{
    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        cb->recordFailure();
    }
    EXPECT_FALSE(cb->allowsCall());
}

TEST_F(CircuitBreakerTest, TransitionsToHalfOpenAfterTimeout)
{
    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        cb->recordFailure();
    }
    EXPECT_EQ(cb->state(), CircuitState::Open);

    std::this_thread::sleep_for(settings.resetTimeout_ + std::chrono::milliseconds(50));

    EXPECT_TRUE(cb->allowsCall());
    EXPECT_EQ(cb->state(), CircuitState::HalfOpen);
}

TEST_F(CircuitBreakerTest, HalfOpenToClosedAfterSuccesses)
{
    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        cb->recordFailure();
    }
    std::this_thread::sleep_for(settings.resetTimeout_ + std::chrono::milliseconds(50));
    cb->allowsCall();

    for (int i = 0; i < settings.halfOpenMaxCalls_; ++i)
    {
        cb->recordSuccess();
    }
    EXPECT_EQ(cb->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakerTest, HalfOpenToOpenOnFailure)
{
    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        cb->recordFailure();
    }
    std::this_thread::sleep_for(settings.resetTimeout_ + std::chrono::milliseconds(50));
    cb->allowsCall();

    cb->recordFailure();
    EXPECT_EQ(cb->state(), CircuitState::Open);
}

TEST_F(CircuitBreakerTest, SuccessResetsFailureCountInClosed)
{
    cb->recordFailure();
    cb->recordFailure();
    cb->recordSuccess();
    cb->recordFailure();
    cb->recordFailure();
    EXPECT_EQ(cb->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakerTest, RecordFailureWithHttpClientError)
{
    cb->recordFailure(HttpClientError::ConnectTimeout);
    cb->recordFailure(HttpClientError::ConnectTimeout);
    EXPECT_EQ(cb->state(), CircuitState::Closed);

    cb->recordFailure(HttpClientError::ConnectTimeout);
    EXPECT_EQ(cb->state(), CircuitState::Open);
}

TEST_F(CircuitBreakerTest, DnsFailureNotCounted)
{
    for (int i = 0; i < settings.failureThreshold_ + 5; ++i)
    {
        cb->recordFailure(HttpClientError::DnsFailed);
    }
    EXPECT_EQ(cb->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakerTest, NoneErrorNotCountedAsFailure)
{
    cb->recordFailure(HttpClientError::None);
    EXPECT_EQ(cb->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakerTest, DoesNotOpenTwice)
{
    for (int i = 0; i < settings.failureThreshold_ + 5; ++i)
    {
        cb->recordFailure();
    }
    EXPECT_EQ(cb->state(), CircuitState::Open);
}