#pragma once

#include "metrics/IMetricsCollector.hpp"
#include <shared_mutex>
#include <unordered_map>
#include <atomic>
#include <array>
#include <memory>
#include <vector>
#include <string>
#include <map>
#include <mutex>

/**
 * @file MetricsCollector.hpp
 * @brief Thread-safe metrics collector implementation
 * @author Anton Tobolkin
 */

struct MetricDefinition
{
    std::string name;
    std::string help;
    std::string type;
};

class MetricsCollector : public IMetricsCollector
{
public:
    /** @brief Default constructor */
    explicit MetricsCollector() = default;

    /**
     * @brief Increment a counter metric
     * @param name Metric name
     * @param labels Key-value labels
     */
    void increment(const std::string &name,
                   const std::map<std::string, std::string> &labels = {}) override;

    /**
     * @brief Set a gauge metric to an arbitrary value
     * @param name Metric name
     * @param value Value to set
     * @param labels Key-value labels
     */
    void set(const std::string &name,
             double value,
             const std::map<std::string, std::string> &labels = {}) override;

    /**
     * @brief Observe a value for histogram distribution
     * @param name Metric name
     * @param value Observed value
     * @param labels Key-value labels
     */
    void observe(const std::string &name,
                 double value,
                 const std::map<std::string, std::string> &labels = {}) override;

    /**
     * @brief Serialize all metrics to Prometheus text format
     * @return String in Prometheus exposition format
     */
    std::string toPrometheusFormat() const override;

private:
    /**
     * @brief Build composite key from name and labels
     * @param name Metric name
     * @param labels Key-value labels
     * @return Composite key string
     */
    std::string buildKey(const std::string &name,
                          const std::map<std::string, std::string> &labels) const;

    /**
     * @brief Format labels as Prometheus label string
     * @param labels Key-value labels
     * @return Formatted string like "key=\"value\",key2=\"value2\""
     */
    static std::string formatLabels(const std::map<std::string, std::string> &labels);

    struct HistogramBucket
    {
        double upperBound = 0.0;
        int64_t count = 0;
        HistogramBucket() = default;
        HistogramBucket(double bound) : upperBound(bound), count(0) {}
    };

    struct HistogramData
    {
        std::vector<HistogramBucket> buckets;
        std::atomic<double> sum{0.0};
        std::atomic<int64_t> count{0};
        std::mutex bucketMutex;
    };

    /** @brief Mutex for thread-safe counters access */
    mutable std::shared_mutex countersMutex_;

    /** @brief Counter metrics storage */
    std::unordered_map<std::string, std::unique_ptr<std::atomic<int64_t>>> counters_;

    /** @brief Mutex for thread-safe gauges access */
    mutable std::shared_mutex gaugesMutex_;

    /** @brief Gauge metrics storage */
    std::unordered_map<std::string, std::unique_ptr<std::atomic<double>>> gauges_;

    /** @brief Mutex for thread-safe histograms access */
    mutable std::shared_mutex histogramsMutex_;

    /** @brief Histogram metrics storage */
    std::unordered_map<std::string, std::unique_ptr<HistogramData>> histograms_;

    /** @brief Default histogram buckets for latency distributions */
    static const std::array<double, 11> kDefaultBuckets;
};