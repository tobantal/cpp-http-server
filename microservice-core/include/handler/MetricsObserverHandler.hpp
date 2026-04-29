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
    /**
     * @brief Construct MetricsObserverHandler
     * @param inner Inner handler to decorate
     * @param metrics Metrics collector
     * @param serviceName Service name for metrics labels
     */
    MetricsObserverHandler(std::shared_ptr<IHttpHandler> inner,
                            std::shared_ptr<IMetricsCollector> metrics,
                            const std::string &serviceName = "");

    /**
     * @brief Handle request and record metrics
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override;

private:
    /** @brief Inner handler */
    std::shared_ptr<IHttpHandler> inner_;

    /** @brief Metrics collector */
    std::shared_ptr<IMetricsCollector> metrics_;

    /** @brief Service name for metrics labels */
    std::string serviceName_;
};