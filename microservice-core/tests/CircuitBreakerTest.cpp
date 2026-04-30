#include <gtest/gtest.h>
#include "circuit/CircuitBreaker.hpp"
#include <thread>
#include <chrono>

class CircuitBreakerTest : public ::testing::Test {
protected:
    CircuitBreaker::Config defaultConfig() {
        return {5, 30, 60};
    }
};

TEST_F(CircuitBreakerTest, InitialStateIsClosed) {
    CircuitBreaker cb(defaultConfig());
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Closed);
}

TEST_F(CircuitBreakerTest, ClosedAllowsRequest) {
    CircuitBreaker cb(defaultConfig());
    EXPECT_TRUE(cb.allowRequest());
}

TEST_F(CircuitBreakerTest, ClosedTransitionsToOpenAfterThreshold) {
    CircuitBreaker cb(defaultConfig());

    for (int i = 0; i < 4; ++i) {
        cb.recordFailure();
        EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Closed);
    }

    cb.recordFailure();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Open);
}

TEST_F(CircuitBreakerTest, OpenBlocksRequest) {
    CircuitBreaker cb(defaultConfig());

    for (int i = 0; i < 5; ++i) {
        cb.recordFailure();
    }
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Open);
    EXPECT_FALSE(cb.allowRequest());
}

TEST_F(CircuitBreakerTest, OpenTransitionsToHalfOpenAfterTimeout) {
    CircuitBreaker::Config config{5, 1, 1};
    CircuitBreaker cb(config);

    for (int i = 0; i < 5; ++i) {
        cb.recordFailure();
    }
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Open);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    EXPECT_TRUE(cb.allowRequest());
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::HalfOpen);
}

TEST_F(CircuitBreakerTest, HalfOpenAllowsOneRequest) {
    CircuitBreaker::Config config{5, 1, 1};
    CircuitBreaker cb(config);

    for (int i = 0; i < 5; ++i) {
        cb.recordFailure();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    cb.allowRequest();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::HalfOpen);
    EXPECT_TRUE(cb.allowRequest());
}

TEST_F(CircuitBreakerTest, HalfOpenSuccessClosesCircuit) {
    CircuitBreaker::Config config{5, 1, 1};
    CircuitBreaker cb(config);

    for (int i = 0; i < 5; ++i) {
        cb.recordFailure();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    cb.allowRequest();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::HalfOpen);

    cb.recordSuccess();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Closed);
}

TEST_F(CircuitBreakerTest, HalfOpenFailureReopensCircuit) {
    CircuitBreaker::Config config{5, 1, 1};
    CircuitBreaker cb(config);

    for (int i = 0; i < 5; ++i) {
        cb.recordFailure();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    cb.allowRequest();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::HalfOpen);

    cb.recordFailure();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Open);
}

TEST_F(CircuitBreakerTest, ClosedWithEnvPrefix) {
    setenv("CB_FAILURE_THRESHOLD", "3", 1);
    setenv("CB_FAILURE_WINDOW_SECONDS", "20", 1);
    setenv("CB_HALF_OPEN_TIMEOUT_SECONDS", "30", 1);

    CircuitBreaker cb("CB_");
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Closed);
    EXPECT_TRUE(cb.allowRequest());

    unsetenv("CB_FAILURE_THRESHOLD");
    unsetenv("CB_FAILURE_WINDOW_SECONDS");
    unsetenv("CB_HALF_OPEN_TIMEOUT_SECONDS");
}

TEST_F(CircuitBreakerTest, ClosedResetsFailureCountAfterSuccess) {
    CircuitBreaker cb(defaultConfig());

    cb.recordFailure();
    cb.recordFailure();
    cb.recordFailure();
    cb.recordSuccess();

    cb.recordFailure();
    cb.recordFailure();
    cb.recordFailure();
    cb.recordFailure();
    EXPECT_EQ(cb.getState(), ICircuitBreaker::State::Open);
}