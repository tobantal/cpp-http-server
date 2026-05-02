#pragma once

#include <chrono>
#include <string>

/**
 * @file ICircuitBreakerSettings.hpp
 * @brief Circuit breaker settings interface
 * @author Anton Tobolkin
 */

/**
 * @class ICircuitBreakerSettings
 * @brief Configuration for circuit breaker behavior
 *
 * Defines thresholds and timeouts for state transitions:
 * - CLOSED → OPEN: when failure count reaches failureThreshold
 * - OPEN → HALF_OPEN: after resetTimeout has elapsed
 * - HALF_OPEN → CLOSED: when halfOpenMaxCalls succeed
 * - HALF_OPEN → OPEN: when a call fails in half-open state
 */
class ICircuitBreakerSettings
{
public:
    virtual ~ICircuitBreakerSettings() = default;

    /**
     * @brief Number of consecutive failures before opening the circuit
     * @return Failure threshold count
     */
    virtual int getFailureThreshold() const = 0;

    /**
     * @brief Time to wait before transitioning from OPEN to HALF_OPEN
     * @return Reset timeout duration
     */
    virtual std::chrono::milliseconds getResetTimeout() const = 0;

    /**
     * @brief Number of successful calls in HALF_OPEN to close the circuit
     * @return Half-open max calls count
     */
    virtual int getHalfOpenMaxCalls() const = 0;
};