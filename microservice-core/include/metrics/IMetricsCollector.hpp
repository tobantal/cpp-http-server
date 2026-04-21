#pragma once

#include <string>
#include <map>
#include <chrono>

/**
 * @file IMetricsCollector.hpp
 * @brief Metrics collector interface for Prometheus-style metrics
 * @author Anton Tobolkin
 */

/**
 * @struct IMetricsCollector
 * @brief Interface for collecting and exposing metrics in Prometheus format
 *
 * Supports three metric types:
 * - Counter: monotonically increasing value (increment only)
 * - Gauge: value that can go up or down (set/observe)
 * - Histogram: distribution of observations (observe with value)
 */
struct IMetricsCollector
{
    virtual ~IMetricsCollector() = default;

    /**
     * @brief Increment a counter metric
     * @param name Metric name (e.g. "http_requests_total")
     * @param labels Key-value labels (e.g. {{"method", "GET"}, {"status", "200"}})
     */
    virtual void increment(const std::string &name,
                           const std::map<std::string, std::string> &labels = {}) = 0;

    /**
     * @brief Set a gauge metric to an arbitrary value
     * @param name Metric name (e.g. "process_uptime_seconds")
     * @param value Value to set
     * @param labels Key-value labels
     */
    virtual void set(const std::string &name,
                     double value,
                     const std::map<std::string, std::string> &labels = {}) = 0;

    /**
     * @brief Observe a value for histogram distribution
     * @param name Metric name (e.g. "http_request_duration_seconds")
     * @param value Observed value
     * @param labels Key-value labels
     */
    virtual void observe(const std::string &name,
                         double value,
                         const std::map<std::string, std::string> &labels = {}) = 0;

    /**
     * @brief Serialize all metrics to Prometheus text format
     * @return String in Prometheus exposition format
     */
    virtual std::string toPrometheusFormat() const = 0;
};