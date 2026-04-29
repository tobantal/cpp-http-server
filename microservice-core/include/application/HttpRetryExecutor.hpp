#pragma once

#include "application/IHttpRetryExecutor.hpp"
#include "application/IHttpRetrySettings.hpp"
#include <thread>
#include <cmath>

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
 * @par Flow
 * 1. Execute func
 * 2. If network error and isRetryOnNetworkErrorEnabled() -> retry
 * 3. If HTTP status in retryable set -> retry
 * 4. If max attempts reached -> return last result
 */
class HttpRetryExecutor : public IHttpRetryExecutor
{
public:
    /**
     * @brief Construct HttpRetryExecutor
     * @param settings HTTP retry configuration
     */
    explicit HttpRetryExecutor(std::shared_ptr<IHttpRetrySettings> settings)
        : settings_(std::move(settings))
    {
    }

    /**
     * @brief Execute callable with retry on network errors and HTTP status
     */
    HttpClientResult execute(const std::function<HttpClientResult()>& func) override
    {
        HttpClientResult lastResult;

        for (int attempt = 1; attempt <= settings_->getMaxAttempts(); ++attempt)
        {
            lastResult = func();

            if (lastResult.ok())
            {
                return lastResult;
            }

            if (!shouldRetryOnNetworkError(lastResult.error))
            {
                return lastResult;
            }

            if (attempt < settings_->getMaxAttempts())
            {
                auto delay = calculateDelay(attempt);
                std::this_thread::sleep_for(delay);
            }
        }

        return lastResult;
    }

    /**
     * @brief Check if HTTP status should trigger retry
     */
    bool shouldRetryOnHttpStatus(int statusCode) const override
    {
        return settings_->getRetryableStatuses().count(statusCode) > 0;
    }

private:
    bool shouldRetryOnNetworkError(HttpClientError error) const
    {
        if (!settings_->isRetryOnNetworkErrorEnabled())
        {
            return false;
        }

        switch (error)
        {
            case HttpClientError::None:
                return false;
            case HttpClientError::DnsFailed:
            case HttpClientError::UnknownError:
                return false;
            default:
                return true;
        }
    }

    std::chrono::milliseconds calculateDelay(int attempt) const
    {
        auto baseDelay = settings_->getBaseDelay();
        auto multiplier = settings_->getMultiplier();
        auto maxDelay = settings_->getMaxDelay();

        double delayMs = baseDelay.count() * std::pow(multiplier, attempt - 1);
        auto calculatedDelay = static_cast<long long>(delayMs);

        if (calculatedDelay > maxDelay.count())
        {
            return maxDelay;
        }

        return std::chrono::milliseconds(calculatedDelay);
    }

    std::shared_ptr<IHttpRetrySettings> settings_;
};