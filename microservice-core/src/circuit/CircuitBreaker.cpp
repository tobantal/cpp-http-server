#include "circuit/CircuitBreaker.hpp"
#include "circuit/CircuitBreakerSettings.hpp"
#include <cstdio>
#include <cstdlib>

CircuitBreaker::CircuitBreaker(Config config)
    : config_(config)
{
}

CircuitBreaker::CircuitBreaker(std::shared_ptr<ICircuitBreakerSettings> settings)
    : config_({
        settings->getFailureThreshold(),
        settings->getFailureWindowSeconds(),
        settings->getHalfOpenTimeoutSeconds()
    })
{
}

CircuitBreaker::CircuitBreaker(const char* envPrefix)
    : config_({
        CircuitBreakerSettings(envPrefix).getFailureThreshold(),
        CircuitBreakerSettings(envPrefix).getFailureWindowSeconds(),
        CircuitBreakerSettings(envPrefix).getHalfOpenTimeoutSeconds()
    })
{
}

ICircuitBreaker::State CircuitBreaker::getState() const
{
    return state_;
}

void CircuitBreaker::recordSuccess()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_ == State::HalfOpen) {
        transitionTo(State::Closed);
    }
}

void CircuitBreaker::recordFailure()
{
    std::lock_guard<std::mutex> lock(mutex_);
    ++failureCount_;
    lastFailureTime_ = std::chrono::steady_clock::now();

    if (state_ == State::Closed) {
        if (failureCount_ >= config_.failureThreshold) {
            transitionTo(State::Open);
        }
    } else if (state_ == State::HalfOpen) {
        transitionTo(State::Open);
    }
}

bool CircuitBreaker::allowRequest()
{
    std::lock_guard<std::mutex> lock(mutex_);

    switch (state_) {
    case State::Closed:
        return true;
    case State::Open:
        if (isHalfOpenTimeoutExpired()) {
            transitionTo(State::HalfOpen);
            return true;
        }
        return false;
    case State::HalfOpen:
        return true;
    }
    return false;
}

void CircuitBreaker::transitionTo(State newState)
{
    state_ = newState;
    stateChangedTime_ = std::chrono::steady_clock::now();

    if (newState == State::Closed) {
        resetFailureCount();
    }
}

bool CircuitBreaker::isFailureWindowExpired() const
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - lastFailureTime_).count();
    return elapsed >= config_.failureWindowSeconds;
}

bool CircuitBreaker::isHalfOpenTimeoutExpired() const
{
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - stateChangedTime_).count();
    return elapsed >= config_.halfOpenTimeoutSeconds;
}

void CircuitBreaker::resetFailureCount()
{
    failureCount_ = 0;
}