#pragma once

#include "ports/output/ITransactionExecutor.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <pqxx/pqxx>
#include <functional>
#include <memory>
#include <string>

/**
 * @file PostgresTransactionExecutor.hpp
 * @brief PostgreSQL transaction executor implementing ITransactionExecutor
 * @author Anton Tobolkin
 */

/**
 * @class PostgresTransactionExecutor
 * @brief Executes database operations within pqxx transactions
 *
 * Wraps pqxx::work in try/commit/catch, logs errors via ILogger, and rethrows.
 * Provides query<T>() for read operations returning a value, and execute() for
 * write operations (void return).
 *
 * @example
 *   PostgresTransactionExecutor executor(logger);
 *   auto conn = pool->connection();
 *
 *   // Read
 *   auto result = executor.query<std::vector<std::string>>(conn,
 *       "UserRepo::findAll",
 *       [](pqxx::work& txn) -> std::vector<std::string> {
 *           pqxx::result r = txn.exec("SELECT name FROM users");
 *           std::vector<std::string> names;
 *           for (auto row : r) names.push_back(row[0].as<std::string>());
 *           return names;
 *       });
 *
 *   // Write
 *   executor.execute(conn, "UserRepo::save",
 *       [&](pqxx::work& txn) {
 *           txn.exec_params("INSERT INTO users(name) VALUES($1)", name);
 *       });
 */
class PostgresTransactionExecutor : public ITransactionExecutor {
public:
    explicit PostgresTransactionExecutor(
        std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>())
        : logger_(std::move(logger))
    {}

    /**
     * @brief Execute a read operation within a transaction
     * @tparam T Return type of the operation
     * @param conn Database connection
     * @param context Description for error logging (e.g. "UserRepo::findById")
     * @param fn Callback receiving pqxx::work, returns T
     * @return Result of the callback
     * @throws std::exception on failure (after logging)
     */
    template<typename T>
    T query(pqxx::connection& conn,
            const std::string& context,
            std::function<T(pqxx::work&)> fn)
    {
        try {
            pqxx::work txn(conn);
            auto result = fn(txn);
            txn.commit();
            return result;
        } catch (const std::exception& e) {
            logger_->log(LogLevel::Error, "TransactionExecutor",
                "[" + context + "] failed: " + e.what());
            throw;
        }
    }

    /**
     * @brief Execute a write operation within a transaction
     * @param conn Database connection
     * @param context Description for error logging
     * @param fn Callback receiving pqxx::work
     * @throws std::exception on failure (after logging)
     */
    void execute(pqxx::connection& conn,
                 const std::string& context,
                 std::function<void(pqxx::work&)> fn);

private:
    std::shared_ptr<ILogger> logger_;
};
