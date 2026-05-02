#pragma once

#include "ports/output/ICircuitBreaker.hpp"
#include "application/ICircuitBreakerSettings.hpp"
#include "ports/output/ILogger.hpp"
#include <chrono>
#include <atomic>
#include <memory>
#include <mutex>

/**
 * @file CircuitBreaker.hpp
 * @brief Circuit breaker implementation
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreaker
 * @brief Thread-safe circuit breaker implementation
 *
 * State machine:
 * - CLOSED → OPEN: after failureThreshold consecutive failures
 * - OPEN → HALF_OPEN: after resetTimeout elapses
 * - HALF_OPEN → CLOSED: after halfOpenMaxCalls consecutive successes
 * - HALF_OPEN → OPEN: on any failure
 *
 * @par Usage
 * @code
 *   auto cb = std::make_shared<CircuitBreaker>(settings, logger);
 *   if (cb->allowsCall()) {
 *       auto result = client.send(req, res);
 *       if (result.ok()) cb->recordSuccess();
 *       else cb->recordFailure(result.error);
 *   }
 * @endcode
 */
class CircuitBreaker : public ICircuitBreaker
{
public:
    /**
     * @brief Construct CircuitBreaker with settings and logger
     * @param settings Circuit breaker configuration
     * @param logger Logger for state transitions
     */
    explicit CircuitBreaker(
        std::shared_ptr<ICircuitBreakerSettings> settings,
        std::shared_ptr<ILogger> logger);

    bool allowsCall() override;
    void recordSuccess() override;
    void recordFailure() override;
    void recordFailure(HttpClientError error) override;
    CircuitState state() const override;

private:
    void transitionTo(CircuitState newState);
    bool isInOpenTimeout() const;
    bool shouldCountAsFailure(HttpClientError error) const;

    std::shared_ptr<ICircuitBreakerSettings> settings_;
    std::shared_ptr<ILogger> logger_;

    mutable std::mutex mutex_;
    std::atomic<CircuitState> state_{CircuitState::Closed};
    std::atomic<int> failureCount_{0};
    std::atomic<int> successCount_{0};
    std::chrono::steady_clock::time_point openedAt_;
};