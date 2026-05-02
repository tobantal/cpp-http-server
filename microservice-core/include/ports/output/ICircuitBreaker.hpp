#pragma once

#include "domain/HttpClientError.hpp"
#include <chrono>
#include <string>

/**
 * @file ICircuitBreaker.hpp
 * @brief Circuit breaker port interface
 * @author Anton Tobolkin
 */

/**
 * @enum CircuitState
 * @brief Circuit breaker states
 */
enum class CircuitState : uint8_t
{
    Closed,
    Open,
    HalfOpen
};

/**
 * @class ICircuitBreaker
 * @brief Interface for circuit breaker pattern
 *
 * Tracks failure count and transitions between states:
 * - CLOSED: normal operation, calls pass through
 * - OPEN: all calls fail fast, no requests are made
 * - HALF_OPEN: limited calls allowed to test if service recovered
 *
 * Thread safety: implementations must be thread-safe.
 */
class ICircuitBreaker
{
public:
    virtual ~ICircuitBreaker() = default;

    /**
     * @brief Check if a call is allowed based on current circuit state
     * @return true if call is allowed (CLOSED or HALF_OPEN with remaining permits)
     */
    virtual bool allowsCall() = 0;

    /**
     * @brief Record a successful call
     *
     * In HALF_OPEN state, success count increments toward closing the circuit.
     */
    virtual void recordSuccess() = 0;

    /**
     * @brief Record a failed call
     *
     * In CLOSED state, increments failure count toward opening the circuit.
     * In HALF_OPEN state, immediately reopens the circuit.
     */
    virtual void recordFailure() = 0;

    /**
     * @brief Record a failure with HTTP-specific error
     * @param error HTTP client error type
     *
     * Some errors (e.g., DNS failure) may not count as circuit failures
     * depending on implementation configuration.
     */
    virtual void recordFailure(HttpClientError error) = 0;

    /**
     * @brief Get current circuit state
     * @return Current state (Closed, Open, HalfOpen)
     */
    virtual CircuitState state() const = 0;
};