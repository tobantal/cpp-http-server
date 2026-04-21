#include "handler/MetricsObserverHandler.hpp"
#include <chrono>

/**
 * @file MetricsObserverHandler.cpp
 * @brief Metrics observer decorator implementation
 * @author Anton Tobolkin
 */

MetricsObserverHandler::MetricsObserverHandler(std::shared_ptr<IHttpHandler> inner,
                                                 std::shared_ptr<IMetricsCollector> metrics,
                                                 const std::string &serviceName)
    : inner_(std::move(inner)), metrics_(std::move(metrics)), serviceName_(serviceName)
{
}

void MetricsObserverHandler::handle(IRequest &req, IResponse &res)
{
    auto start = std::chrono::steady_clock::now();

    inner_->handle(req, res);

    auto end = std::chrono::steady_clock::now();
    double durationSec = std::chrono::duration<double>(end - start).count();

    std::map<std::string, std::string> labels = {
        {"method", req.getMethod()},
        {"path", req.getPathPattern().empty() ? req.getPath() : req.getPathPattern()},
        {"status", std::to_string(res.getStatus())}};

    if (!serviceName_.empty())
    {
        labels["service"] = serviceName_;
    }

    metrics_->increment("http_requests_total", labels);
    metrics_->observe("http_request_duration_seconds", durationSec, labels);
}