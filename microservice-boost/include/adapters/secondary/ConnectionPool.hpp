#pragma once

#include "ports/output/IConnectionPool.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <pqxx/pqxx>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

/**
 * @file ConnectionPool.hpp
 * @brief Thread-safe PostgreSQL connection pool
 * @author Anton Tobolkin
 */

/**
 * @class PooledConnection
 * @brief RAII handle for a borrowed connection from ConnectionPool
 *
 * Returns the connection to the pool on destruction. Move-only.
 */
class PooledConnection {
public:
    PooledConnection() : conn_(nullptr), pool_(nullptr) {}

    PooledConnection(pqxx::connection* conn, class ConnectionPool* pool)
        : conn_(conn), pool_(pool) {}

    PooledConnection(PooledConnection&& other) noexcept
        : conn_(other.conn_), pool_(other.pool_) {
        other.conn_ = nullptr;
        other.pool_ = nullptr;
    }

    PooledConnection& operator=(PooledConnection&& other) noexcept {
        if (this != &other) {
            reset();
            conn_ = other.conn_;
            pool_ = other.pool_;
            other.conn_ = nullptr;
            other.pool_ = nullptr;
        }
        return *this;
    }

    PooledConnection(const PooledConnection&) = delete;
    PooledConnection& operator=(const PooledConnection&) = delete;

    ~PooledConnection();

    /** @brief Access the underlying pqxx connection */
    pqxx::connection& get() { return *conn_; }

    /** @brief Implicit conversion for use as pqxx::connection& */
    operator pqxx::connection&() { return *conn_; }

    /** @brief Whether this handle holds a valid connection */
    bool valid() const { return conn_ != nullptr; }

private:
    void reset();

    pqxx::connection* conn_;
    ConnectionPool* pool_;
};

/**
 * @class ConnectionPool
 * @brief Thread-safe PostgreSQL connection pool implementing IConnectionPool
 *
 * Creates minConnections at initialization, grows up to maxConnections on demand.
 * Connections are returned to the pool via PooledConnection RAII.
 *
 * Implements IConnectionPool (which extends IShutdown) for graceful shutdown
 * and health check integration.
 *
 * @example
 *   auto pool = std::make_shared<ConnectionPool>(connString, Config{2, 10}, logger);
 *   auto conn = pool->connection();
 *   pqxx::work txn(conn.get());
 *   // ... use txn ...
 *   txn.commit();
 *   // conn returns to pool when it goes out of scope
 */
class ConnectionPool : public IConnectionPool {
public:
    struct Config {
        size_t minConnections;
        size_t maxConnections;
    };

    static constexpr Config defaultConfig{2, 10};

    /**
     * @brief Construct connection pool
     * @param connString PostgreSQL connection string
     * @param config Pool sizing (min/max)
     * @param logger Logger (default=NullLogger)
     */
    ConnectionPool(const std::string& connString,
                   Config config = defaultConfig,
                   std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());

    ~ConnectionPool() override;

    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

    /** @brief Checkout a connection from the pool (blocking if all busy) */
    PooledConnection connection();

    // --- IShutdown ---
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    /** @brief Component name for ShutdownManager logging */
    std::string name() const override { return "ConnectionPool"; }

    // --- IConnectionPool ---
    size_t available() const override;
    size_t size() const override;
    bool isAlive() const override;

private:
    friend class PooledConnection;

    void returnConnection(pqxx::connection* conn);
    std::unique_ptr<pqxx::connection> createConnection();

    std::string connString_;
    Config config_;
    std::shared_ptr<ILogger> logger_;

    mutable std::mutex mutex_;
    std::condition_variable cv_;

    std::queue<pqxx::connection*> available_;
    size_t totalCreated_ = 0;
    bool shutdown_ = false;
};