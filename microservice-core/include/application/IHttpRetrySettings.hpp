#pragma once

#include "application/IRetrySettings.hpp"
#include <set>
#include <string>
#include <sstream>

/**
 * @file IHttpRetrySettings.hpp
 * @brief HTTP-specific retry settings interface
 * @author Anton Tobolkin
 */

/**
 * @class IHttpRetrySettings
 * @brief HTTP-specific retry configuration
 *
 * Extends IRetrySettings with HTTP-specific settings:
 * retryable status codes and network error retry flag.
 *
 * @par Usage
 * @code
 *   auto httpSettings = std::make_shared<HttpRetrySettings>("HTTP");
 * @endcode
 */
class IHttpRetrySettings : public IRetrySettings
{
public:
    ~IHttpRetrySettings() override = default;

    /**
     * @brief Get HTTP status codes that trigger retry
     * @return Set of retryable HTTP status codes (default: {500, 502, 503, 504})
     */
    virtual const std::set<int>& getRetryableStatuses() const = 0;

    /**
     * @brief Check if network errors should trigger retry
     * @return true if retry on network errors enabled (default: true)
     */
    virtual bool isRetryOnNetworkErrorEnabled() const = 0;
};