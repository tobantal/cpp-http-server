#pragma once

#include "ports/output/IHealthCheck.hpp"
#include "ports/output/IConnectionPool.hpp"
#include <memory>
#include <string>

/**
 * @file PostgresHealthCheck.hpp
 * @brief Health check for PostgreSQL via IConnectionPool
 * @author Anton Tobolkin
 */

/**
 * @class PostgresHealthCheck
 * @brief IHealthCheck implementation for PostgreSQL database connectivity
 *
 * Uses IConnectionPool::isAlive() to verify database connectivity.
 * Reports pool stats in the status message.
 */
class PostgresHealthCheck : public IHealthCheck
{
public:
    /**
     * @brief Construct PostgresHealthCheck
     * @param pool Connection pool to check
     */
    explicit PostgresHealthCheck(std::shared_ptr<IConnectionPool> pool);

    /**
     * @brief Check PostgreSQL health
     * @return HealthStatus with name "database" and pool stats
     */
    HealthStatus check() const override;

private:
    std::shared_ptr<IConnectionPool> pool_;
};
