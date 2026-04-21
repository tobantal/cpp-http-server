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
    explicit MetricsHandler(std::shared_ptr<IMetricsCollector> metrics);

    void handle(IRequest &req, IResponse &res) override;

private:
    std::shared_ptr<IMetricsCollector> metrics_;
};