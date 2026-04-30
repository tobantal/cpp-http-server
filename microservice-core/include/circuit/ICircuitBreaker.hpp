#pragma once

#include <chrono>

/**
 * @file ICircuitBreaker.hpp
 * @brief Interface for Circuit Breaker pattern
 * @author Anton Tobolkin
 */

/**
 * @class ICircuitBreaker
 * @brief Interface for Circuit Breaker state management
 *
 * The Circuit Breaker pattern prevents cascading failures by wrapping
 * calls to external services and monitoring their health.
 *
 * <b>State Machine:</b>
 * - <b>CLOSED:</b> Normal operation. Requests pass through.
 *   Failures are counted. When threshold is reached → OPEN.
 * - <b>OPEN:</b> Fail-fast mode. Requests are blocked.
 *   After timeout → HALF_OPEN (probe request allowed).
 * - <b>HALF_OPEN:</b> Probe mode. One request allowed to test health.
 *   Success → CLOSED (reset). Failure → OPEN.
 *
 * <b>Usage:</b>
 * @code
 * CircuitBreaker cb(Config{5, 30, 60});
 *
 * if (cb.allowRequest()) {
 *     try {
 *         auto result = callService();
 *         cb.recordSuccess();
 *         return result;
 *     } catch (...) {
 *         cb.recordFailure();
 *         throw;
 *     }
 * }
 * // Circuit open - fail fast
 * throw ServiceUnavailableException();
 * @endcode
 */
class ICircuitBreaker
{
public:
    virtual ~ICircuitBreaker() = default;

    /**
     * @enum State
     * @brief Circuit Breaker states
     */
    enum class State
    {
        /**
         * @brief Normal operation - requests pass through
         *
         * Failures are counted. When failureCount >= threshold
         * within failureWindow, transitions to OPEN.
         */
        Closed,

        /**
         * @brief Fail-fast mode - requests are blocked
         *
         * All requests are rejected immediately.
         * After halfOpenTimeout seconds, transitions to HALF_OPEN
         * to test if the service has recovered.
         */
        Open,

        /**
         * @brief Probe mode - one request allowed to test health
         *
         * Single request is allowed to test service health.
         * Success: transition to CLOSED (circuit reset).
         * Failure: transition back to OPEN (circuit re-opened).
         */
        HalfOpen
    };

    /**
     * @brief Get current circuit state
     * @return Current state (Closed, Open, or HalfOpen)
     */
    virtual State getState() const = 0;

    /**
     * @brief Record successful request
     *
     * Call on successful response from the protected service.
     * In CLOSED state: resets failure count.
     * In HALF_OPEN state: transitions to CLOSED (circuit reset).
     * In OPEN state: no effect (waiting for timeout).
     */
    virtual void recordSuccess() = 0;

    /**
     * @brief Record failed request
     *
     * Call on any failure (exception, timeout, 5xx response).
     * In CLOSED state: increments failure count.
     *   If failureCount >= threshold → transitions to OPEN.
     * In HALF_OPEN state: immediately transitions to OPEN.
     * In OPEN state: no effect (already open).
     */
    virtual void recordFailure() = 0;

    /**
     * @brief Check if request is allowed to proceed
     * @return true if request may proceed, false if circuit is open
     *
     * CLOSED: always returns true.
     * OPEN: checks if halfOpenTimeout expired.
     *   If expired → transitions to HALF_OPEN and returns true.
     *   If not expired → returns false.
     * HALF_OPEN: always returns true (probe request).
     */
    virtual bool allowRequest() = 0;
};