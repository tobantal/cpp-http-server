#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "metrics/IMetricsCollector.hpp"
#include <memory>

/**
 * @file MetricsHandler.hpp
 * @brief HTTP handler that exposes metrics in Prometheus format at GET /metrics
 * @author Anton Tobolkin
 */

/**
 * @class MetricsHandler
 * @brief IHttpHandler that serves Prometheus-format metrics at GET /metrics
 *
 * Returns metrics via IMetricsCollector::toPrometheusFormat() with
 * Content-Type: text/plain; version=0.0.4; charset=utf-8
 */
class MetricsHandler : public IHttpHandler
{
public:
    /**
     * @brief Construct MetricsHandler
     * @param metrics Metrics collector instance
     */
    explicit MetricsHandler(std::shared_ptr<IMetricsCollector> metrics);

    /**
     * @brief Handle GET /metrics request
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override;

private:
    /** @brief Metrics collector */
    std::shared_ptr<IMetricsCollector> metrics_;
};