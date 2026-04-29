#pragma once

#include "application/IHttpRetryExecutor.hpp"
#include "application/IHttpRetrySettings.hpp"
#include "ports/output/ILogger.hpp"
#include <memory>

/**
 * @file HttpRetryExecutor.hpp
 * @brief HTTP retry executor implementation
 * @author Anton Tobolkin
 */

/**
 * @class HttpRetryExecutor
 * @brief Implements HTTP retry with status code and network error checking
 *
 * Executes operations with retry on:
 * - Network errors (if isRetryOnNetworkErrorEnabled)
 * - HTTP 5xx status codes
 *
 * Logs warning on each failed retry attempt in format "N/M".
 */
class HttpRetryExecutor : public IHttpRetryExecutor
{
public:
    /**
     * @brief Construct HttpRetryExecutor
     * @param settings HTTP retry configuration
     * @param logger Logger for warning messages
     */
    explicit HttpRetryExecutor(
        std::shared_ptr<IHttpRetrySettings> settings,
        std::shared_ptr<ILogger> logger);

    HttpClientResult execute(const std::function<HttpClientResult()>& func) override;
    bool shouldRetryOnHttpStatus(int statusCode) const override;

private:
    std::chrono::milliseconds calculateDelay(int attempt) const;
    bool shouldRetryOnNetworkError(HttpClientError error) const;

    std::shared_ptr<IHttpRetrySettings> settings_;
    std::shared_ptr<ILogger> logger_;
};