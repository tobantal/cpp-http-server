#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#include "adapters/primary/CircuitBreakingHttpClient.hpp"
#include "domain/CircuitBreaker.hpp"
#include "application/CircuitBreakerSettings.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"
#include "adapters/secondary/NullLogger.hpp"

class MockCircuitBreakerSettings : public ICircuitBreakerSettings
{
public:
    int failureThreshold_ = 3;
    std::chrono::milliseconds resetTimeout_{100};
    int halfOpenMaxCalls_ = 2;

    int getFailureThreshold() const override { return failureThreshold_; }
    std::chrono::milliseconds getResetTimeout() const override { return resetTimeout_; }
    int getHalfOpenMaxCalls() const override { return halfOpenMaxCalls_; }
};

class MockHttpClient : public IHttpClient
{
public:
    HttpClientResult sendResult{HttpClientError::None, ""};
    int callCount = 0;

    HttpClientResult send(const IRequest&, IResponse&) override
    {
        ++callCount;
        return sendResult;
    }
};

class CircuitBreakingHttpClientTest : public ::testing::Test
{
protected:
    MockCircuitBreakerSettings settings;
    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>();
    std::shared_ptr<MockHttpClient> mockClient = std::make_shared<MockHttpClient>();
    std::shared_ptr<CircuitBreaker> circuitBreaker;
    std::shared_ptr<CircuitBreakingHttpClient> client;

    void SetUp() override
    {
        circuitBreaker = std::make_shared<CircuitBreaker>(
            std::make_shared<MockCircuitBreakerSettings>(settings), logger);
        client = std::make_shared<CircuitBreakingHttpClient>(
            mockClient, circuitBreaker, logger);
    }
};

TEST_F(CircuitBreakingHttpClientTest, SendsRequestInClosedState)
{
    SimpleRequest request;
    SimpleResponse response;
    auto result = client->send(request, response);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(mockClient->callCount, 1);
}

TEST_F(CircuitBreakingHttpClientTest, RecordsSuccessOnOkResult)
{
    mockClient->sendResult = HttpClientResult{HttpClientError::None, ""};
    SimpleRequest request;
    SimpleResponse response;
    client->send(request, response);
    EXPECT_EQ(circuitBreaker->state(), CircuitState::Closed);
}

TEST_F(CircuitBreakingHttpClientTest, RecordsFailureOnError)
{
    mockClient->sendResult = HttpClientResult{HttpClientError::ConnectTimeout, "timeout"};
    SimpleRequest request;
    SimpleResponse response;

    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        client->send(request, response);
    }
    EXPECT_EQ(circuitBreaker->state(), CircuitState::Open);
}

TEST_F(CircuitBreakingHttpClientTest, RejectsRequestWhenCircuitIsOpen)
{
    mockClient->sendResult = HttpClientResult{HttpClientError::ConnectTimeout, "timeout"};
    SimpleRequest request;
    SimpleResponse response;

    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        client->send(request, response);
    }
    EXPECT_EQ(circuitBreaker->state(), CircuitState::Open);

    auto result = client->send(request, response);
    EXPECT_FALSE(result.ok());
    EXPECT_EQ(mockClient->callCount, settings.failureThreshold_);
}

TEST_F(CircuitBreakingHttpClientTest, AllowsRequestAfterResetTimeout)
{
    mockClient->sendResult = HttpClientResult{HttpClientError::ConnectTimeout, "timeout"};
    SimpleRequest request;
    SimpleResponse response;

    for (int i = 0; i < settings.failureThreshold_; ++i)
    {
        client->send(request, response);
    }
    EXPECT_EQ(circuitBreaker->state(), CircuitState::Open);

    std::this_thread::sleep_for(settings.resetTimeout_ + std::chrono::milliseconds(50));

    mockClient->sendResult = HttpClientResult{HttpClientError::None, ""};
    auto result = client->send(request, response);
    EXPECT_TRUE(result.ok());
    EXPECT_EQ(circuitBreaker->state(), CircuitState::HalfOpen);
}