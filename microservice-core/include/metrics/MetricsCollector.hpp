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
    explicit MetricsCollector() = default;

    void increment(const std::string &name,
                   const std::map<std::string, std::string> &labels = {}) override;

    void set(const std::string &name,
             double value,
             const std::map<std::string, std::string> &labels = {}) override;

    void observe(const std::string &name,
                 double value,
                 const std::map<std::string, std::string> &labels = {}) override;

    std::string toPrometheusFormat() const override;

private:
    std::string buildKey(const std::string &name,
                          const std::map<std::string, std::string> &labels) const;

    static std::string formatLabels(const std::map<std::string, std::string> &labels);

    struct HistogramBucket
    {
        double upperBound = 0.0;
        int64_t count = 0;
        HistogramBucket();
        HistogramBucket(double bound);
    };

    struct HistogramData
    {
        std::vector<HistogramBucket> buckets;
        std::atomic<double> sum{0.0};
        std::atomic<int64_t> count{0};
        std::mutex bucketMutex;
    };

    mutable std::shared_mutex countersMutex_;
    std::unordered_map<std::string, std::unique_ptr<std::atomic<int64_t>>> counters_;

    mutable std::shared_mutex gaugesMutex_;
    std::unordered_map<std::string, std::unique_ptr<std::atomic<double>>> gauges_;

    mutable std::shared_mutex histogramsMutex_;
    std::unordered_map<std::string, std::unique_ptr<HistogramData>> histograms_;

    static const std::array<double, 11> kDefaultBuckets;
};