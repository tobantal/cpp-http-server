#pragma once

#include "ports/output/IHttpClient.hpp"
#include "ports/output/ICircuitBreaker.hpp"
#include "ports/output/ILogger.hpp"

/**
 * @file CircuitBreakingHttpClient.hpp
 * @brief IHttpClient decorator with circuit breaker protection
 * @author Anton Tobolkin
 */

/**
 * @class CircuitBreakingHttpClient
 * @brief Decorator that adds circuit breaker logic to IHttpClient
 *
 * When the circuit is OPEN, send() returns an error immediately
 * without calling the inner client. When CLOSED or HALF_OPEN,
 * requests pass through and results are recorded.
 */
class CircuitBreakingHttpClient : public IHttpClient
{
public:
    /**
     * @brief Construct CircuitBreakingHttpClient
     * @param inner Inner HTTP client to wrap
     * @param circuitBreaker Circuit breaker instance
     * @param logger Logger for circuit events
     */
    CircuitBreakingHttpClient(
        std::shared_ptr<IHttpClient> inner,
        std::shared_ptr<ICircuitBreaker> circuitBreaker,
        std::shared_ptr<ILogger> logger);

    /**
     * @brief Send HTTP request through circuit breaker
     *
     * If circuit is OPEN, returns error immediately.
     * Otherwise, delegates to inner client and records success/failure.
     */
    HttpClientResult send(const IRequest& request, IResponse& response) override;

private:
    std::shared_ptr<IHttpClient> inner_;
    std::shared_ptr<ICircuitBreaker> circuitBreaker_;
    std::shared_ptr<ILogger> logger_;
};