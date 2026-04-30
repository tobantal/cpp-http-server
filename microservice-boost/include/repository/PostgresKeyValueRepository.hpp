#pragma once

#include "repository/IRepository.hpp"
#include "repository/KeyValueEntity.hpp"
#include "adapters/secondary/ConnectionPool.hpp"
#include "adapters/secondary/PostgresTransactionExecutor.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <memory>
#include <string>

/**
 * @file PostgresKeyValueRepository.hpp
 * @brief Key-value repository backed by PostgreSQL
 * @author Anton Tobolkin
 */

/**
 * @class PostgresKeyValueRepository
 * @brief PostgreSQL implementation of IRepository<KeyValueEntity>
 *
 * Stores key-value pairs in a configurable table.
 * Uses ConnectionPool for connection management and
 * PostgresTransactionExecutor for transaction handling.
 *
 * @example
 * @code
 * auto pool = std::make_shared<ConnectionPool>(connStr, Config{2, 10}, logger);
 * PostgresKeyValueRepository repo(pool, logger, "kv_store");
 * repo.ensureSchema();  // Create table if not exists
 *
 * KeyValueEntity kv{"user:1", R"({"name":"Alice"})"};
 * repo.save(kv);
 * auto found = repo.findById("user:1");
 * auto all = repo.findAll();
 * repo.removeById("user:1");
 * @endcode
 */
class PostgresKeyValueRepository : public IRepository<KeyValueEntity>
{
public:
    /**
     * @brief Construct repository
     * @param pool Connection pool
     * @param logger Logger (default=NullLogger)
     * @param tableName Table name (default="key_value_store")
     */
    PostgresKeyValueRepository(std::shared_ptr<ConnectionPool> pool,
                              std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>(),
                              const std::string &tableName = "key_value_store");

    std::optional<KeyValueEntity> findById(const std::string &id) override;
    std::vector<KeyValueEntity> findAll() override;
    KeyValueEntity save(const KeyValueEntity &entity) override;
    std::vector<KeyValueEntity> saveAll(const std::vector<KeyValueEntity> &entities) override;
    bool removeById(const std::string &id) override;

    /**
     * @brief Create the key-value table if it does not exist
     *
     * Safe to call multiple times (idempotent). Creates:
     *   - Table with columns: id VARCHAR PRIMARY KEY, value TEXT NOT NULL
     */
    void ensureSchema();

private:
    std::shared_ptr<ConnectionPool> pool_;
    PostgresTransactionExecutor executor_;
    std::shared_ptr<ILogger> logger_;
    std::string tableName_;
};