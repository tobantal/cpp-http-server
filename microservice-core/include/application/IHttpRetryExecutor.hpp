#pragma once

#include "application/IRetryPolicy.hpp"
#include "application/IRetrySettings.hpp"
#include "application/IExecutorPolicy.hpp"
#include "domain/HttpClientError.hpp"
#include <memory>
#include <thread>
#include <cmath>

/**
 * @file IHttpRetryExecutor.hpp
 * @brief HTTP-specific retry executor interface
 * @author Anton Tobolkin
 */

/**
 * @class IHttpRetryExecutor
 * @brief HTTP retry executor with status code checking
 *
 * Extends IExecutorPolicy<HttpClientResult> with HTTP-specific
 * retry conditions (status codes).
 *
 * @par Usage
 * @code
 *   auto httpExecutor = std::make_shared<HttpRetryExecutor>(settings, policy);
 *   auto result = httpExecutor->execute([&]() { return httpClient->send(req, res); });
 * @endcode
 */
class IHttpRetryExecutor : public IExecutorPolicy<HttpClientResult>
{
public:
    ~IHttpRetryExecutor() override = default;

    /**
     * @brief Check if HTTP status should trigger retry
     * @param statusCode HTTP status code (e.g., 500, 503)
     * @return true if status is retryable (in configured set)
     *
     * 5xx server errors are typically transient and retryable.
     * 4xx client errors indicate request problems and are NOT retryable.
     */
    virtual bool shouldRetryOnHttpStatus(int statusCode) const = 0;
};