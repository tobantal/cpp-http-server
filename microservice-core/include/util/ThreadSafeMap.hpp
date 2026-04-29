#pragma once

#include <unordered_map>
#include <shared_mutex>
#include <memory>
#include <mutex>
#include <vector>

/**
 * @file ThreadSafeMap.hpp
 * @brief Thread-safe map with read-write locking
 * @author Anton Tobolkin
 */

/**
 * @template K Key type
 * @template V Value type (shared_ptr)
 */
template <typename K, typename V>
class ThreadSafeMap
{
public:
    /** @brief Default constructor */
    ThreadSafeMap() = default;

    /**
     * @brief Insert key-value pair
     * @param key Key
     * @param value Value (shared_ptr)
     */
    void insert(const K &key, const std::shared_ptr<V> &value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_[key] = value;
    }

    /**
     * @brief Find value by key
     * @param key Key to find
     * @return Value or nullptr if not found
     */
    std::shared_ptr<V> find(const K &key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = map_.find(key);
        return (it != map_.end()) ? it->second : nullptr;
    }

    /**
     * @brief Check if key exists
     * @param key Key to check
     * @return true if key exists
     */
    bool contains(const K &key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.find(key) != map_.end();
    }

    /**
     * @brief Remove key
     * @param key Key to remove
     */
    void remove(const K &key)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.erase(key);
    }

    /** @brief Clear all entries */
    void clear()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        map_.clear();
    }

    /**
     * @brief Get all values
     * @return Vector of all values
     */
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
    /** @brief Mutex for thread-safe access */
    mutable std::shared_mutex mutex_;

    /** @brief Internal map storage */
    std::unordered_map<K, std::shared_ptr<V>> map_;
};
