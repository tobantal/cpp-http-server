#pragma once

#include "ports/output/IHttpClient.hpp"
#include "application/IHttpRetryExecutor.hpp"

/**
 * @file RetryingHttpClient.hpp
 * @brief IHttpClient decorator with retry logic
 * @author Anton Tobolkin
 */

/**
 * @class RetryingHttpClient
 * @brief Decorator that adds retry logic to IHttpClient
 *
 * Uses IHttpRetryExecutor for retry logic with HTTP-specific
 * conditions (status codes, network errors).
 */
class RetryingHttpClient : public IHttpClient
{
public:
    /**
     * @brief Construct RetryingHttpClient
     * @param inner Inner HTTP client to wrap
     * @param executor HTTP retry executor
     */
    RetryingHttpClient(
        std::shared_ptr<IHttpClient> inner,
        std::shared_ptr<IHttpRetryExecutor> executor)
        : inner_(std::move(inner))
        , executor_(std::move(executor))
    {
    }

    /**
     * @brief Send HTTP request with retry
     */
    HttpClientResult send(const IRequest& request, IResponse& response) override
    {
        return executor_->execute([this, &request, &response]() {
            return inner_->send(request, response);
        });
    }

private:
    std::shared_ptr<IHttpClient> inner_;
    std::shared_ptr<IHttpRetryExecutor> executor_;
};