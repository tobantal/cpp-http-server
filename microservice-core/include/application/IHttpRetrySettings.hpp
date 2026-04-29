#pragma once

#include "application/IRetrySettings.hpp"
#include <chrono>
#include <set>
#include <string>

/**
 * @file IHttpRetrySettings.hpp
 * @brief HTTP-specific retry settings interface
 * @author Anton Tobolkin
 */

/**
 * @class IHttpRetrySettings
 * @brief HTTP retry configuration interface
 *
 * Extends IRetrySettings with HTTP-specific retry conditions.
 */
class IHttpRetrySettings : public IRetrySettings
{
public:
    ~IHttpRetrySettings() override = default;

    /**
     * @brief Get HTTP status codes that should trigger retry
     * @return Set of retryable HTTP status codes (typically 5xx)
     */
    virtual const std::set<int>& getRetryableStatuses() const = 0;

    /**
     * @brief Check if network errors should trigger retry
     * @return true if network errors are retryable
     */
    virtual bool isRetryOnNetworkErrorEnabled() const = 0;
};