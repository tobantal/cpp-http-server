#pragma once

#include <cstdint>

/**
 * @file ICircuitBreakerSettings.hpp
 * @brief Interface for Circuit Breaker configuration
 * @author Anton Tobolkin
 */

/**
 * @class ICircuitBreakerSettings
 * @brief Interface for Circuit Breaker settings
 */
class ICircuitBreakerSettings
{
public:
    virtual ~ICircuitBreakerSettings() = default;

    virtual uint32_t getFailureThreshold() const = 0;
    virtual uint32_t getFailureWindowSeconds() const = 0;
    virtual uint32_t getHalfOpenTimeoutSeconds() const = 0;
};