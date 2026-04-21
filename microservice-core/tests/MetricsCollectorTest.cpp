#include <gtest/gtest.h>
#include "metrics/MetricsCollector.hpp"
#include "handler/MetricsHandler.hpp"
#include "handler/MetricsObserverHandler.hpp"
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"

/**
 * @file MetricsCollectorTest.cpp
 * @brief Unit tests for IMetricsCollector, MetricsCollector, MetricsHandler, MetricsObserverHandler
 */

// =============================================================================
// MetricsCollector — Counter tests
// =============================================================================

TEST(MetricsCollectorTest, IncrementCounterNoLabels)
{
    MetricsCollector mc;
    mc.increment("http_requests_total");
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find("# TYPE http_requests_total counter"), std::string::npos);
    EXPECT_NE(output.find("http_requests_total 1"), std::string::npos);
}

TEST(MetricsCollectorTest, IncrementCounterWithLabels)
{
    MetricsCollector mc;
    mc.increment("http_requests_total", {{"method", "GET"}, {"status", "200"}});
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find(R"(http_requests_total{method="GET",status="200"} 1)"), std::string::npos);
}

TEST(MetricsCollectorTest, IncrementCounterMultipleTimes)
{
    MetricsCollector mc;
    mc.increment("requests_total");
    mc.increment("requests_total");
    mc.increment("requests_total");
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find("requests_total 3"), std::string::npos);
}

TEST(MetricsCollectorTest, IncrementCounterDifferentLabels)
{
    MetricsCollector mc;
    mc.increment("http_requests_total", {{"status", "200"}});
    mc.increment("http_requests_total", {{"status", "500"}});
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find(R"({status="200"} 1)"), std::string::npos);
    EXPECT_NE(output.find(R"({status="500"} 1)"), std::string::npos);
}

// =============================================================================
// MetricsCollector — Gauge tests
// =============================================================================

TEST(MetricsCollectorTest, SetGaugeNoLabels)
{
    MetricsCollector mc;
    mc.set("process_uptime_seconds", 42.5);
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find("# TYPE process_uptime_seconds gauge"), std::string::npos);
    EXPECT_NE(output.find("process_uptime_seconds 42.5"), std::string::npos);
}

TEST(MetricsCollectorTest, SetGaugeOverwrite)
{
    MetricsCollector mc;
    mc.set("temperature", 10.0);
    mc.set("temperature", 20.0);
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find("temperature 20"), std::string::npos);
}

// =============================================================================
// MetricsCollector — Histogram tests
// =============================================================================

TEST(MetricsCollectorTest, ObserveHistogram)
{
    MetricsCollector mc;
    mc.observe("http_request_duration_seconds", 0.1);
    auto output = mc.toPrometheusFormat();
    EXPECT_NE(output.find("# TYPE http_request_duration_seconds histogram"), std::string::npos);
    EXPECT_NE(output.find("http_request_duration_seconds_sum 0.1"), std::string::npos);
    EXPECT_NE(output.find("http_request_duration_seconds_count 1"), std::string::npos);
    EXPECT_NE(output.find("http_request_duration_seconds_bucket{le=\"+Inf\"} 1"), std::string::npos);
}

// =============================================================================
// MetricsCollector — Prometheus format
// =============================================================================

TEST(MetricsCollectorTest, EmptyCollectorProducesNoOutput)
{
    MetricsCollector mc;
    auto output = mc.toPrometheusFormat();
    EXPECT_TRUE(output.empty());
}

// =============================================================================
// MetricsHandler — /metrics endpoint
// =============================================================================

TEST(MetricsHandlerTest, ReturnsPrometheusFormat)
{
    auto metrics = std::make_shared<MetricsCollector>();
    metrics->increment("test_total", {{"env", "prod"}});
    MetricsHandler handler(metrics);

    SimpleRequest req("GET", "/metrics", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_NE(res.getBody().find("# TYPE test_total counter"), std::string::npos);
}

TEST(MetricsHandlerTest, ContentTypeIsPrometheus)
{
    auto metrics = std::make_shared<MetricsCollector>();
    MetricsHandler handler(metrics);

    SimpleRequest req("GET", "/metrics", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    auto ct = res.getHeader("Content-Type");
    ASSERT_TRUE(ct.has_value());
    EXPECT_EQ(*ct, "text/plain; version=0.0.4; charset=utf-8");
}

// =============================================================================
// MetricsObserverHandler — decorator
// =============================================================================

TEST(MetricsObserverHandlerTest, RecordsHttpRequestsTotal)
{
    auto metrics = std::make_shared<MetricsCollector>();
    auto healthHandler = std::make_shared<HealthHandler>();
    MetricsObserverHandler observer(healthHandler, metrics);

    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    req.setPathPattern("/health");
    SimpleResponse res;
    observer.handle(req, res);

    auto output = metrics->toPrometheusFormat();
    EXPECT_NE(output.find("# TYPE http_requests_total counter"), std::string::npos);
    EXPECT_NE(output.find(R"(method="GET")"), std::string::npos);
    EXPECT_NE(output.find(R"(status="200")"), std::string::npos);
}

TEST(MetricsObserverHandlerTest, RecordsHistogram)
{
    auto metrics = std::make_shared<MetricsCollector>();
    auto healthHandler = std::make_shared<HealthHandler>();
    MetricsObserverHandler observer(healthHandler, metrics);

    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    observer.handle(req, res);

    auto output = metrics->toPrometheusFormat();
    EXPECT_NE(output.find("# TYPE http_request_duration_seconds histogram"), std::string::npos);
}

TEST(MetricsObserverHandlerTest, RecordsWithServiceName)
{
    auto metrics = std::make_shared<MetricsCollector>();
    auto healthHandler = std::make_shared<HealthHandler>();
    MetricsObserverHandler observer(healthHandler, metrics, "trading");

    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    observer.handle(req, res);

    auto output = metrics->toPrometheusFormat();
    EXPECT_NE(output.find(R"(service="trading")"), std::string::npos);
}

TEST(MetricsObserverHandlerTest, PassesThroughToInnerHandler)
{
    auto metrics = std::make_shared<MetricsCollector>();
    auto healthHandler = std::make_shared<HealthHandler>();
    MetricsObserverHandler observer(healthHandler, metrics);

    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    observer.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_NE(res.getBody().find("ok"), std::string::npos);
}