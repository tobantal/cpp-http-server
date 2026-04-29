#pragma once

#include <chrono>
#include <functional>

/**
 * @file IRetryPolicy.hpp
 * @brief Generic retry policy interface
 * @author Anton Tobolkin
 */

/**
 * @class IRetryPolicy
 * @brief Generic interface for retry timing and conditions
 *
 * Provides generic retry mechanism with exponential backoff.
 * Does NOT handle service-specific error types (network, HTTP, etc.)
 * - those are handled by concrete IExecutorPolicy implementations.
 *
 * @par Usage
 * @code
 *   auto policy = std::make_shared<RetryPolicy>(settings);
 *   auto delay = policy->getDelayForAttempt(2); // 2000ms
 * @endcode
 */
class IRetryPolicy
{
public:
    virtual ~IRetryPolicy() = default;

    /**
     * @brief Calculate delay for given attempt number
     * @param attempt Attempt number (1-based)
     * @return Delay in milliseconds before next retry
     *
     * Formula: delay = min(baseDelay * multiplier^(attempt-1), maxDelay)
     *
     * Example: baseDelay=1000, multiplier=2.0, maxDelay=30000
     *   attempt 1: 1000ms
     *   attempt 2: 2000ms
     *   attempt 3: 4000ms
     *   attempt 4: 8000ms
     */
    virtual std::chrono::milliseconds getDelayForAttempt(int attempt) const = 0;
};