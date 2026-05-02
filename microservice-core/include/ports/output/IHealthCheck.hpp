#pragma once

#include <string>

/**
 * @file IHealthCheck.hpp
 * @brief Output port for health check providers
 * @author Anton Tobolkin
 */

/**
 * @struct HealthStatus
 * @brief Result of a single health check
 */
struct HealthStatus
{
    /** @brief Name of the dependency being checked (e.g., "database", "rabbitmq") */
    std::string name;
    /** @brief true if the dependency is healthy */
    bool healthy;
    /** @brief Optional detail message (e.g., "Connection pool: 3/10 available") */
    std::string message;
};

/**
 * @class IHealthCheck
 * @brief Interface for health check providers
 *
 * Implementations check the health of a specific dependency
 * (database, message broker, external service, etc.).
 * Registered in HealthHandler for aggregated /health endpoint.
 */
class IHealthCheck
{
public:
    virtual ~IHealthCheck() = default;

    /**
     * @brief Perform health check
     * @return HealthStatus with name, healthy flag, and optional message
     */
    virtual HealthStatus check() const = 0;
};
