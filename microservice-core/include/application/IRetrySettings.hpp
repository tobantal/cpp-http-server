#pragma once

#include <chrono>

/**
 * @file IRetrySettings.hpp
 * @brief Generic retry timing configuration
 * @author Anton Tobolkin
 */

/**
 * @class IRetrySettings
 * @brief Base configuration interface for retry timing
 *
 * Provides generic retry timing configuration that works for any service.
 * Defines only timing parameters - no error-specific logic.
 *
 * @par Usage
 * @code
 *   auto settings = std::make_shared<RetrySettings>("HTTP");
 * @endcode
 */
class IRetrySettings
{
public:
    virtual ~IRetrySettings() = default;

    /**
     * @brief Get maximum retry attempts
     * @return Max attempts count (default: 3)
     *
     * Limits number of retry attempts to prevent infinite loops.
     */
    virtual int getMaxAttempts() const = 0;

    /**
     * @brief Get base delay between retry attempts
     * @return Base delay in milliseconds (default: 1000)
     *
     * Initial delay before first retry.
     * Formula: delay = baseDelay * multiplier^(attempt-1)
     */
    virtual std::chrono::milliseconds getBaseDelay() const = 0;

    /**
     * @brief Get backoff multiplier
     * @return Multiplier value (default: 2.0)
     *
     * With multiplier=2.0: 1s → 2s → 4s → 8s...
     */
    virtual double getMultiplier() const = 0;

    /**
     * @brief Get maximum delay cap
     * @return Max delay in milliseconds (default: 30000)
     *
     * Caps exponential backoff to prevent excessive wait times.
     */
    virtual std::chrono::milliseconds getMaxDelay() const = 0;
};