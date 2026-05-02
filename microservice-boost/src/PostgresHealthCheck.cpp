#include "adapters/primary/PostgresHealthCheck.hpp"

PostgresHealthCheck::PostgresHealthCheck(std::shared_ptr<IConnectionPool> pool)
    : pool_(std::move(pool))
{
}

HealthStatus PostgresHealthCheck::check() const
{
    HealthStatus status;
    status.name = "database";

    bool alive = pool_->isAlive();
    status.healthy = alive;

    size_t available = pool_->available();
    size_t size = pool_->size();
    status.message = "Pool: " + std::to_string(available) + "/" + std::to_string(size) + " available";

    return status;
}
