#include "domain/CircuitBreaker.hpp"

CircuitBreaker::CircuitBreaker(
    std::shared_ptr<ICircuitBreakerSettings> settings,
    std::shared_ptr<ILogger> logger)
    : settings_(std::move(settings))
    , logger_(std::move(logger))
{
}

bool CircuitBreaker::allowsCall()
{
    auto current = state_.load();
    if (current == CircuitState::Closed)
    {
        return true;
    }
    if (current == CircuitState::Open)
    {
        if (isInOpenTimeout())
        {
            std::lock_guard<std::mutex> lock(mutex_);
            transitionTo(CircuitState::HalfOpen);
            successCount_.store(0);
            return true;
        }
        return false;
    }
    // HalfOpen: allow limited calls
    return true;
}

void CircuitBreaker::recordSuccess()
{
    auto current = state_.load();
    if (current == CircuitState::HalfOpen)
    {
        int count = successCount_.fetch_add(1) + 1;
        if (count >= settings_->getHalfOpenMaxCalls())
        {
            std::lock_guard<std::mutex> lock(mutex_);
            failureCount_.store(0);
            transitionTo(CircuitState::Closed);
        }
    }
    else if (current == CircuitState::Closed)
    {
        failureCount_.store(0);
    }
}

void CircuitBreaker::recordFailure()
{
    auto current = state_.load();
    if (current == CircuitState::Closed)
    {
        int count = failureCount_.fetch_add(1) + 1;
        if (count >= settings_->getFailureThreshold())
        {
            std::lock_guard<std::mutex> lock(mutex_);
            transitionTo(CircuitState::Open);
        }
    }
    else if (current == CircuitState::HalfOpen)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        transitionTo(CircuitState::Open);
    }
}

void CircuitBreaker::recordFailure(HttpClientError error)
{
    if (shouldCountAsFailure(error))
    {
        recordFailure();
    }
}

CircuitState CircuitBreaker::state() const
{
    return state_.load();
}

void CircuitBreaker::transitionTo(CircuitState newState)
{
    CircuitState oldState = state_.load();
    if (oldState == newState) return;

    state_.store(newState);

    if (newState == CircuitState::Open)
    {
        openedAt_ = std::chrono::steady_clock::now();
    }

    const char* oldName = oldState == CircuitState::Closed ? "Closed"
        : oldState == CircuitState::Open ? "Open" : "HalfOpen";
    const char* newName = newState == CircuitState::Closed ? "Closed"
        : newState == CircuitState::Open ? "Open" : "HalfOpen";

    logger_->log(LogLevel::Info, "CircuitBreaker",
        std::string(oldName) + " -> " + newName);
}

bool CircuitBreaker::isInOpenTimeout() const
{
    auto elapsed = std::chrono::steady_clock::now() - openedAt_;
    return elapsed >= settings_->getResetTimeout();
}

bool CircuitBreaker::shouldCountAsFailure(HttpClientError error) const
{
    if (error == HttpClientError::None) return false;
    // DNS failures are infrastructure issues, not downstream failures
    if (error == HttpClientError::DnsFailed) return false;
    return true;
}