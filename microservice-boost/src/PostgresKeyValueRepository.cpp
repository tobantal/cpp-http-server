#include "repository/PostgresKeyValueRepository.hpp"

PostgresKeyValueRepository::PostgresKeyValueRepository(std::shared_ptr<ConnectionPool> pool,
                                                     std::shared_ptr<ILogger> logger,
                                                     const std::string &tableName)
    : pool_(std::move(pool))
    , executor_(std::move(logger))
    , tableName_(tableName)
{
}

std::optional<KeyValueEntity> PostgresKeyValueRepository::findById(const std::string &id)
{
    auto conn = pool_->connection();
    if (!conn.valid())
    {
        return std::nullopt;
    }

    return executor_.query<std::optional<KeyValueEntity>>(conn.get(),
        "PostgresKeyValueRepository::findById",
        [&](pqxx::work &txn) -> std::optional<KeyValueEntity> {
            pqxx::result r = txn.exec_params(
                "SELECT id, value FROM " + tableName_ + " WHERE id = $1", id);
            if (r.empty())
            {
                return std::nullopt;
            }
            KeyValueEntity entity;
            entity.id = r[0][0].as<std::string>();
            entity.value = r[0][1].as<std::string>();
            return entity;
        });
}

std::vector<KeyValueEntity> PostgresKeyValueRepository::findAll()
{
    auto conn = pool_->connection();
    if (!conn.valid())
    {
        return {};
    }

    return executor_.query<std::vector<KeyValueEntity>>(conn.get(),
        "PostgresKeyValueRepository::findAll",
        [&](pqxx::work &txn) -> std::vector<KeyValueEntity> {
            pqxx::result r = txn.exec("SELECT id, value FROM " + tableName_);
            std::vector<KeyValueEntity> result;
            result.reserve(r.size());
            for (const auto &row : r)
            {
                KeyValueEntity entity;
                entity.id = row[0].as<std::string>();
                entity.value = row[1].as<std::string>();
                result.push_back(entity);
            }
            return result;
        });
}

KeyValueEntity PostgresKeyValueRepository::save(const KeyValueEntity &entity)
{
    auto conn = pool_->connection();

    executor_.execute(conn.get(),
        "PostgresKeyValueRepository::save",
        [&](pqxx::work &txn) {
            txn.exec_params(
                "INSERT INTO " + tableName_ + " (id, value) VALUES ($1, $2) "
                "ON CONFLICT (id) DO UPDATE SET value = EXCLUDED.value",
                entity.id, entity.value);
        });

    return entity;
}

bool PostgresKeyValueRepository::removeById(const std::string &id)
{
    auto conn = pool_->connection();

    int deleted = executor_.query<int>(conn.get(),
        "PostgresKeyValueRepository::removeById",
        [&](pqxx::work &txn) -> int {
            pqxx::result r = txn.exec_params(
                "DELETE FROM " + tableName_ + " WHERE id = $1", id);
            return r.affected_rows();
        });

    return deleted > 0;
}

void PostgresKeyValueRepository::ensureSchema()
{
    auto conn = pool_->connection();

    executor_.execute(conn.get(),
        "PostgresKeyValueRepository::ensureSchema",
        [&](pqxx::work &txn) {
            txn.exec(
                "CREATE TABLE IF NOT EXISTS " + tableName_ +
                " (id VARCHAR(255) PRIMARY KEY, value TEXT NOT NULL)");
        });
}