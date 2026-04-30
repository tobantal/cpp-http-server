#pragma once

#include "repository/IRepository.hpp"
#include "repository/KeyValueEntity.hpp"
#include <unordered_map>
#include <mutex>
#include <algorithm>

/**
 * @file InMemoryKeyValueRepository.hpp
 * @brief In-memory key-value repository for unit tests and caching
 * @author Anton Tobolkin
 */

/**
 * @class InMemoryKeyValueRepository
 * @brief Thread-safe in-memory implementation of IRepository<KeyValueEntity>
 *
 * Stores key-value pairs in an unordered_map. Suitable for:
 * - Unit tests (test double replacing PostgresKeyValueRepository)
 * - Caching layer (fast lookup before hitting the database)
 *
 * All operations are mutex-protected for thread safety.
 */
class InMemoryKeyValueRepository : public IRepository<KeyValueEntity>
{
public:
    std::optional<KeyValueEntity> findById(const std::string &id) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = store_.find(id);
        if (it == store_.end())
        {
            return std::nullopt;
        }
        return it->second;
    }

    std::vector<KeyValueEntity> findAll() override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<KeyValueEntity> result;
        result.reserve(store_.size());
        for (const auto &[key, entity] : store_)
        {
            result.push_back(entity);
        }
        return result;
    }

    KeyValueEntity save(const KeyValueEntity &entity) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        store_[entity.id] = entity;
        return entity;
    }

    void saveAll(const std::vector<KeyValueEntity> &entities) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto &entity : entities)
        {
            store_[entity.id] = entity;
        }
    }

    bool removeById(const std::string &id) override
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return store_.erase(id) > 0;
    }

    /**
     * @brief Clear all stored entities
     */
    void clear()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        store_.clear();
    }

    /**
     * @brief Get the number of stored entities
     */
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return store_.size();
    }

private:
    std::unordered_map<std::string, KeyValueEntity> store_;
    mutable std::mutex mutex_;
};