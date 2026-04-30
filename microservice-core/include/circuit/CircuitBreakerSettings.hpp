#pragma once

#include "circuit/ICircuitBreakerSettings.hpp"
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <string>

/**
 * @file CircuitBreakerSettings.hpp
 * @brief Circuit Breaker settings implementation from ENV
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreakerSettings
 * @brief Circuit Breaker configuration loaded from environment variables
 *
 * Loads configuration from ENV with prefix:
 * - `{PREFIX}_FAILURE_THRESHOLD` — failures before opening (default: 5)
 * - `{PREFIX}_FAILURE_WINDOW_SECONDS` — window to count failures (default: 30)
 * - `{PREFIX}_HALF_OPEN_TIMEOUT_SECONDS` — time before probe (default: 60)
 *
 * @par Example
 * @code
 * CircuitBreakerSettings settings("HTTP_BROKER_");
 * uint32_t threshold = settings.getFailureThreshold();  // from HTTP_BROKER_FAILURE_THRESHOLD env
 * @endcode
 */
class CircuitBreakerSettings : public ICircuitBreakerSettings
{
public:
    static constexpr uint32_t kDefaultFailureThreshold = 5;
    static constexpr uint32_t kDefaultFailureWindowSeconds = 30;
    static constexpr uint32_t kDefaultHalfOpenTimeoutSeconds = 60;

    /**
     * @brief Construct settings with ENV prefix
     * @param prefix Environment variable prefix
     *
     * Reads from env:
     * - `{prefix}FAILURE_THRESHOLD` (default: 5)
     * - `{prefix}FAILURE_WINDOW_SECONDS` (default: 30)
     * - `{prefix}HALF_OPEN_TIMEOUT_SECONDS` (default: 60)
     */
    explicit CircuitBreakerSettings(const char* prefix)
        : prefix_(prefix)
    {
    }

    /**
     * @brief Get failure threshold
     * @return Number of failures to trip the circuit
     */
    uint32_t getFailureThreshold() const override
    {
        return getEnvUint32("FAILURE_THRESHOLD", kDefaultFailureThreshold);
    }

    /**
     * @brief Get failure window in seconds
     * @return Time window to count failures
     */
    uint32_t getFailureWindowSeconds() const override
    {
        return getEnvUint32("FAILURE_WINDOW_SECONDS", kDefaultFailureWindowSeconds);
    }

    /**
     * @brief Get half-open timeout in seconds
     * @return Time before transitioning from OPEN to HALF_OPEN
     */
    uint32_t getHalfOpenTimeoutSeconds() const override
    {
        return getEnvUint32("HALF_OPEN_TIMEOUT_SECONDS", kDefaultHalfOpenTimeoutSeconds);
    }

private:
    const char* prefix_;

    uint32_t getEnvUint32(const char* name, uint32_t defaultValue) const
    {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s%s", prefix_, name);
        const char* value = std::getenv(buffer);
        if (value) {
            try {
                return std::stoul(value);
            } catch (const std::exception&) {
                return defaultValue;
            }
        }
        return defaultValue;
    }
};