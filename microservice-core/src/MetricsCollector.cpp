#include "metrics/MetricsCollector.hpp"
#include <sstream>
#include <algorithm>

/**
 * @file MetricsCollector.cpp
 * @brief Thread-safe metrics collector implementation
 * @author Anton Tobolkin
 */

const std::array<double, 11> MetricsCollector::kDefaultBuckets = {
    0.005, 0.01, 0.025, 0.05, 0.1, 0.25, 0.5, 1.0, 2.5, 5.0, 10.0};

MetricsCollector::HistogramBucket::HistogramBucket() = default;

MetricsCollector::HistogramBucket::HistogramBucket(double bound) : upperBound(bound), count(0) {}

void MetricsCollector::increment(const std::string &name,
                                  const std::map<std::string, std::string> &labels)
{
    std::string key = buildKey(name, labels);
    std::shared_lock<std::shared_mutex> lock(countersMutex_);
    auto it = counters_.find(key);
    if (it != counters_.end())
    {
        it->second->fetch_add(1);
        return;
    }
    lock.unlock();

    std::unique_lock<std::shared_mutex> ulock(countersMutex_);
    it = counters_.find(key);
    if (it != counters_.end())
    {
        it->second->fetch_add(1);
        return;
    }
    auto counter = std::make_unique<std::atomic<int64_t>>(1);
    counters_[key] = std::move(counter);
}

void MetricsCollector::set(const std::string &name,
                             double value,
                             const std::map<std::string, std::string> &labels)
{
    std::string key = buildKey(name, labels);
    std::shared_lock<std::shared_mutex> lock(gaugesMutex_);
    auto it = gauges_.find(key);
    if (it != gauges_.end())
    {
        it->second->store(value);
        return;
    }
    lock.unlock();

    std::unique_lock<std::shared_mutex> ulock(gaugesMutex_);
    it = gauges_.find(key);
    if (it != gauges_.end())
    {
        it->second->store(value);
        return;
    }
    auto gauge = std::make_unique<std::atomic<double>>(value);
    gauges_[key] = std::move(gauge);
}

void MetricsCollector::observe(const std::string &name,
                                 double value,
                                 const std::map<std::string, std::string> &labels)
{
    std::string key = buildKey(name, labels);
    std::unique_lock<std::shared_mutex> lock(histogramsMutex_);
    auto it = histograms_.find(key);
    if (it == histograms_.end())
    {
        auto hist = std::make_unique<HistogramData>();
        for (double bound : kDefaultBuckets)
        {
            hist->buckets.emplace_back(bound);
        }
        hist->sum.store(value);
        hist->count.store(1);
        for (auto &bucket : hist->buckets)
        {
            if (value <= bucket.upperBound)
            {
                bucket.count = 1;
            }
        }
        histograms_[key] = std::move(hist);
    }
    else
    {
        auto &hist = it->second;
        double oldSum = hist->sum.load();
        while (!hist->sum.compare_exchange_weak(oldSum, oldSum + value))
        {
            oldSum = hist->sum.load();
        }
        hist->count.fetch_add(1);
        std::lock_guard<std::mutex> bucketLock(hist->bucketMutex);
        for (auto &bucket : hist->buckets)
        {
            if (value <= bucket.upperBound)
            {
                bucket.count++;
            }
        }
    }
}

std::string MetricsCollector::toPrometheusFormat() const
{
    std::ostringstream output;

    std::shared_lock<std::shared_mutex> cLock(countersMutex_);
    for (const auto &[key, value] : counters_)
    {
        std::string name = key.substr(0, key.find('{'));
        output << "# TYPE " << name << " counter\n";
        output << key << " " << value->load() << "\n";
    }
    cLock.unlock();

    std::shared_lock<std::shared_mutex> gLock(gaugesMutex_);
    for (const auto &[key, value] : gauges_)
    {
        std::string name = key.substr(0, key.find('{'));
        output << "# TYPE " << name << " gauge\n";
        output << key << " " << value->load() << "\n";
    }
    gLock.unlock();

    std::shared_lock<std::shared_mutex> hLock(histogramsMutex_);
    for (const auto &[key, hist] : histograms_)
    {
        std::string name = key.substr(0, key.find('{'));
        output << "# TYPE " << name << " histogram\n";
        {
            std::lock_guard<std::mutex> bucketLock(hist->bucketMutex);
            for (const auto &bucket : hist->buckets)
            {
                output << name << "_bucket{le=\"" << bucket.upperBound << "\"} "
                       << bucket.count << "\n";
            }
        }
        output << name << "_bucket{le=\"+Inf\"} " << hist->count.load() << "\n";
        output << name << "_sum " << hist->sum.load() << "\n";
        output << name << "_count " << hist->count.load() << "\n";
    }
    hLock.unlock();

    return output.str();
}

std::string MetricsCollector::buildKey(const std::string &name,
                                         const std::map<std::string, std::string> &labels) const
{
    if (labels.empty())
    {
        return name;
    }
    return name + "{" + formatLabels(labels) + "}";
}

std::string MetricsCollector::formatLabels(const std::map<std::string, std::string> &labels)
{
    std::string result;
    bool first = true;
    for (const auto &[k, v] : labels)
    {
        if (!first)
        {
            result.append(",");
        }
        result.append(k);
        result.append("=\"");
        result.append(v);
        result.append("\"");
        first = false;
    }
    return result;
}