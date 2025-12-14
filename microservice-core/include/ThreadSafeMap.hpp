#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <mutex>

template <typename K, typename V>
class ThreadSafeMap
{
public:
    ThreadSafeMap() = default;

    void insert(const K &key, const std::shared_ptr<V> &value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_); // ← UNIQUE_LOCK для WRITE!
        map_[key] = value;
    }

    std::shared_ptr<V> find(const K &key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_); // ← shared_lock для READ
        auto it = map_.find(key);
        return (it != map_.end()) ? it->second : nullptr;
    }

    bool contains(const K &key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_); // ← shared_lock для READ
        return map_.find(key) != map_.end();
    }

    void remove(const K &key)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_); // ← UNIQUE_LOCK для WRITE!
        map_.erase(key);
    }

    void clear()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_); // ← UNIQUE_LOCK для WRITE!
        map_.clear();
    }

    std::vector<std::shared_ptr<V>> getAll() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        std::vector<std::shared_ptr<V>> result;
        result.reserve(map_.size());
        for (const auto &[key, value] : map_)
        {
            result.push_back(value);
        }
        return result;
    }

private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<K, std::shared_ptr<V>> map_;
};
