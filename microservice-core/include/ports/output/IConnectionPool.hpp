#pragma once

#include "ports/output/IShutdown.hpp"
#include <cstddef>

/**
 * @file IConnectionPool.hpp
 * @brief Output port for database connection pool lifecycle
 * @author Anton Tobolkin
 */

/**
 * @class IConnectionPool
 * @brief Interface for database connection pool lifecycle and health
 *
 * Inherits IShutdown so the pool can participate in graceful shutdown
 * via ShutdownManager (LIFO order). Does NOT expose connection borrowing —
 * that is pqxx-specific and belongs in the concrete ConnectionPool adapter.
 *
 * Repositories depend on the concrete ConnectionPool (microservice-boost)
 * which provides connection(). IConnectionPool is for health checks,
 * DI wiring, and shutdown coordination.
 */
class IConnectionPool : public IShutdown {
public:
    /**
     * @brief Number of idle connections available for checkout
     */
    virtual size_t available() const = 0;

    /**
     * @brief Total number of connections created (active + idle)
     */
    virtual size_t size() const = 0;

    /**
     * @brief Check database connectivity
     * @return true if a connection can execute a query (e.g. SELECT 1)
     */
    virtual bool isAlive() const = 0;
};