#include "handler/MetricsHandler.hpp"

/**
 * @file MetricsHandler.cpp
 * @brief Prometheus metrics endpoint handler implementation
 * @author Anton Tobolkin
 */

MetricsHandler::MetricsHandler(std::shared_ptr<IMetricsCollector> metrics)
    : metrics_(std::move(metrics))
{
}

void MetricsHandler::handle(IRequest & /*req*/, IResponse &res)
{
    res.setResult(200, "text/plain; version=0.0.4; charset=utf-8",
                  metrics_->toPrometheusFormat());
}