#include "adapters/secondary/ConnectionPool.hpp"
#include "ports/output/ILogger.hpp"
#include <stdexcept>

// --- PooledConnection ---

PooledConnection::~PooledConnection() {
    reset();
}

void PooledConnection::reset() {
    if (conn_ && pool_) {
        pool_->returnConnection(conn_);
    }
    conn_ = nullptr;
    pool_ = nullptr;
}

PooledConnection::PooledConnection(PooledConnection&& other) noexcept
    : conn_(other.conn_), pool_(other.pool_) {
    other.conn_ = nullptr;
    other.pool_ = nullptr;
}

PooledConnection& PooledConnection::operator=(PooledConnection&& other) noexcept {
    if (this != &other) {
        reset();
        conn_ = other.conn_;
        pool_ = other.pool_;
        other.conn_ = nullptr;
        other.pool_ = nullptr;
    }
    return *this;
}

pqxx::connection& PooledConnection::get() {
    return *conn_;
}

PooledConnection::operator pqxx::connection&() {
    return *conn_;
}

bool PooledConnection::valid() const {
    return conn_ != nullptr;
}

// --- ConnectionPool ---

ConnectionPool::ConnectionPool(const std::string& connString,
                               Config config,
                               std::shared_ptr<ILogger> logger)
    : connString_(connString)
    , config_(config)
    , logger_(std::move(logger))
{
    logger_->log(LogLevel::Info, "ConnectionPool",
        "Initializing pool with min=" + std::to_string(config_.minConnections) +
        " max=" + std::to_string(config_.maxConnections));

    for (size_t i = 0; i < config_.minConnections; ++i) {
        try {
            auto conn = createConnection();
            available_.push(conn.release());
        } catch (const std::exception& e) {
            logger_->log(LogLevel::Error, "ConnectionPool",
                "Failed to create initial connection: " + std::string(e.what()));
            throw;
        }
    }

    logger_->log(LogLevel::Info, "ConnectionPool",
        std::to_string(available_.size()) + " connections ready");
}

ConnectionPool::~ConnectionPool() {
    shutdown();

    std::lock_guard<std::mutex> lock(mutex_);
    while (!available_.empty()) {
        auto* conn = available_.front();
        available_.pop();
        delete conn;
    }
}

PooledConnection ConnectionPool::connection() {
    std::unique_lock<std::mutex> lock(mutex_);

    while (!available_.empty()) {
        auto* conn = available_.front();
        available_.pop();

        if (conn->is_open()) {
            return PooledConnection(conn, this);
        }

        logger_->log(LogLevel::Warn, "ConnectionPool", "Discarding dead connection");
        delete conn;
        --totalCreated_;
    }

    if (totalCreated_ < config_.maxConnections) {
        lock.unlock();
        std::unique_ptr<pqxx::connection> conn;
        try {
            conn = createConnection();
        } catch (const std::exception& e) {
            logger_->log(LogLevel::Error, "ConnectionPool",
                "Failed to create connection: " + std::string(e.what()));
            throw;
        }
        lock.lock();
        return PooledConnection(conn.release(), this);
    }

    logger_->log(LogLevel::Warn, "ConnectionPool",
        "All " + std::to_string(config_.maxConnections) + " connections in use, waiting");

    cv_.wait(lock, [this] { return !available_.empty() || shutdown_; });

    if (shutdown_) {
        throw std::runtime_error("ConnectionPool: pool is shut down");
    }

    auto* conn = available_.front();
    available_.pop();

    if (!conn->is_open()) {
        delete conn;
        --totalCreated_;
        lock.unlock();
        return connection();
    }

    return PooledConnection(conn, this);
}

void ConnectionPool::returnConnection(pqxx::connection* conn) {
    if (!conn) return;

    std::lock_guard<std::mutex> lock(mutex_);

    if (shutdown_) {
        delete conn;
        --totalCreated_;
        return;
    }

    if (!conn->is_open()) {
        delete conn;
        --totalCreated_;
        logger_->log(LogLevel::Warn, "ConnectionPool", "Discarded dead connection on return");
        return;
    }

    available_.push(conn);
    cv_.notify_one();
}

void ConnectionPool::shutdown(std::chrono::milliseconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_ = true;
    cv_.notify_all();
}

size_t ConnectionPool::available() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return available_.size();
}

size_t ConnectionPool::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return totalCreated_;
}

bool ConnectionPool::isAlive() const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (available_.empty()) {
        return totalCreated_ > 0;
    }

    auto* conn = available_.front();
    return conn->is_open();
}

std::string ConnectionPool::name() const {
    return "ConnectionPool";
}

std::unique_ptr<pqxx::connection> ConnectionPool::createConnection() {
    auto conn = std::make_unique<pqxx::connection>(connString_);
    ++totalCreated_;
    logger_->log(LogLevel::Info, "ConnectionPool",
        "Created connection, total=" + std::to_string(totalCreated_));
    return conn;
}