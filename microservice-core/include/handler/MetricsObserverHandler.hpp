#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "metrics/IMetricsCollector.hpp"
#include <memory>
#include <string>

/**
 * @file MetricsObserverHandler.hpp
 * @brief Decorator that records HTTP request metrics after handler chain completes
 * @author Anton Tobolkin
 */

/**
 * @class MetricsObserverHandler
 * @brief IHttpHandler decorator that records http_requests_total after inner handler completes
 *
 * Wraps any IHttpHandler. After inner->handle() completes, reads res.getStatus()
 * and records http_requests_total{method="GET",path="/api/v1/orders",status="200"}.
 * Also records http_request_duration_seconds histogram.
 */
class MetricsObserverHandler : public IHttpHandler
{
public:
    MetricsObserverHandler(std::shared_ptr<IHttpHandler> inner,
                            std::shared_ptr<IMetricsCollector> metrics,
                            const std::string &serviceName = "");

    void handle(IRequest &req, IResponse &res) override;

private:
    std::shared_ptr<IHttpHandler> inner_;
    std::shared_ptr<IMetricsCollector> metrics_;
    std::string serviceName_;
};