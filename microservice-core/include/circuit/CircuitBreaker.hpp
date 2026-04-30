#pragma once

#include "circuit/ICircuitBreaker.hpp"
#include "circuit/ICircuitBreakerSettings.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>

/**
 * @file CircuitBreaker.hpp
 * @brief Circuit Breaker implementation
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreaker
 * @brief Thread-safe Circuit Breaker implementation
 *
 * Implements the Circuit Breaker pattern to prevent cascading failures
 * when calling external services.
 *
 * <b>State Transitions:</b>
 *
 * @code
 *  CLOSED ──(failures >= threshold)──> OPEN
 *    ^                                │
 *    │                                │
 *  (success)                      (timeout)
 *    │                                │
 *    │                                ▼
 *  HALF_OPEN <──(timeout expired)── OPEN
 * @endcode
 *
 * <b>Configuration:</b>
 * Can be configured via:
 * - Programmatic Config struct
 * - Environment variables with prefix
 * - ICircuitBreakerSettings interface
 *
 * <b>Thread Safety:</b>
 * All public methods are thread-safe via internal mutex.
 *
 * @par Example
 * @code
 * CircuitBreaker cb(CircuitBreaker::Config{5, 30, 60});
 *
 * if (!cb.allowRequest()) {
 *     throw ServiceUnavailableException("Circuit is open");
 * }
 *
 * try {
 *     auto result = httpClient->send(request, response);
 *     cb.recordSuccess();
 *     return result;
 * } catch (...) {
 *     cb.recordFailure();
 *     throw;
 * }
 * @endcode
 */
class CircuitBreaker : public ICircuitBreaker
{
public:
    /**
     * @struct Config
     * @brief Circuit Breaker configuration parameters
     */
    struct Config
    {
        /**
         * @brief Number of failures required to trip the circuit
         *
         * When failureCount reaches this threshold within failureWindow,
         * circuit transitions from CLOSED to OPEN.
         *
         * @default 5
         */
        uint32_t failureThreshold = 5;

        /**
         * @brief Time window in seconds to count failures
         *
         * Only failures within this window are counted.
         * Failures older than this window are ignored.
         *
         * @default 30
         */
        uint32_t failureWindowSeconds = 30;

        /**
         * @brief Time in seconds before transitioning from OPEN to HALF_OPEN
         *
         * After this timeout in OPEN state, circuit allows one probe request
         * (half-open state) to test if the service has recovered.
         *
         * @default 60
         */
        uint32_t halfOpenTimeoutSeconds = 60;
    };

    /**
     * @brief Construct CircuitBreaker with explicit config
     * @param config Configuration parameters (failureThreshold, failureWindowSeconds, halfOpenTimeoutSeconds)
     */
    explicit CircuitBreaker(Config config);

    /**
     * @brief Construct CircuitBreaker with settings interface
     * @param settings Pointer to ICircuitBreakerSettings
     *
     * @par Example
     * @code
     * auto settings = std::make_shared<CircuitBreakerSettings>("HTTP_BROKER_");
     * CircuitBreaker cb(settings);
     * @endcode
     */
    explicit CircuitBreaker(std::shared_ptr<ICircuitBreakerSettings> settings);

    /**
     * @brief Construct CircuitBreaker with ENV-based configuration
     * @param envPrefix Prefix for environment variables
     *
     * Reads configuration from environment:
     * - `{PREFIX}FAILURE_THRESHOLD` (default: 5)
     * - `{PREFIX}FAILURE_WINDOW_SECONDS` (default: 30)
     * - `{PREFIX}HALF_OPEN_TIMEOUT_SECONDS` (default: 60)
     *
     * @par Example
     * @code
     * CircuitBreaker cb("HTTP_CLIENT_");
     * // reads: HTTP_CLIENT_FAILURE_THRESHOLD, HTTP_CLIENT_FAILURE_WINDOW_SECONDS,
     * //        HTTP_CLIENT_HALF_OPEN_TIMEOUT_SECONDS
     * @endcode
     */
    explicit CircuitBreaker(const char* envPrefix);

    /**
     * @copydoc ICircuitBreaker::getState()
     *
     * Thread-safe: uses internal mutex.
     * @return Current state of the circuit breaker
     */
    State getState() const override;

    /**
     * @copydoc ICircuitBreaker::recordSuccess()
     *
     * Thread-safe: uses internal mutex.
     */
    void recordSuccess() override;

    /**
     * @copydoc ICircuitBreaker::recordFailure()
     *
     * Thread-safe: uses internal mutex.
     */
    void recordFailure() override;

    /**
     * @copydoc ICircuitBreaker::allowRequest()
     *
     * Thread-safe: uses internal mutex.
     * @return true if request is allowed, false if circuit is open
     */
    bool allowRequest() override;

private:
    Config config_;
    State state_ = State::Closed;
    uint32_t failureCount_ = 0;
    std::chrono::steady_clock::time_point lastFailureTime_;
    std::chrono::steady_clock::time_point stateChangedTime_;
    mutable std::mutex mutex_;

    void transitionTo(State newState);
    bool isFailureWindowExpired() const;
    bool isHalfOpenTimeoutExpired() const;
    void resetFailureCount();
};