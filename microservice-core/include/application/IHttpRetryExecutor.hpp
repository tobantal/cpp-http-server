#pragma once

#include "application/IExecutorPolicy.hpp"
#include "domain/HttpClientError.hpp"
#include <memory>

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
 */
class IHttpRetryExecutor : public IExecutorPolicy<HttpClientResult>
{
public:
    ~IHttpRetryExecutor() override = default;

    /**
     * @brief Check if HTTP status should trigger retry
     * @param statusCode HTTP status code (e.g., 500, 503)
     * @return true if status is retryable (in configured set)
     */
    virtual bool shouldRetryOnHttpStatus(int statusCode) const = 0;
};