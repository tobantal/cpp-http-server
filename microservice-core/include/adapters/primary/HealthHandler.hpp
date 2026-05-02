#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "ports/output/IHealthCheck.hpp"
#include <string>
#include <vector>
#include <memory>

/**
 * @file HealthHandler.hpp
 * @brief Health check handler with dependency checks
 * @author Anton Tobolkin
 */

/**
 * @class HealthHandler
 * @brief Aggregated health check endpoint (Spring Boot Actuator analog)
 *
 * Registers IHealthCheck providers and checks all on /health request.
 *
 * Response when all checks pass (HTTP 200):
 *   {"status":"UP","service":"<name>","checks":{"database":{"status":"UP"},"rabbitmq":{"status":"UP"}}}
 *
 * Response when any check fails (HTTP 503):
 *   {"status":"DOWN","service":"<name>","checks":{"database":{"status":"DOWN","message":"..."}"}}
 *
 * If no IHealthCheck providers registered, returns simple:
 *   {"status":"UP","service":"<name>"}
 */
class HealthHandler : public IHttpHandler
{
public:
    /**
     * @brief Construct HealthHandler with service name and optional health checks
     * @param serviceName Name of the service for health response
     * @param checks Vector of health check providers
     */
    HealthHandler(const std::string& serviceName = "unknown",
                  std::vector<std::shared_ptr<IHealthCheck>> checks = {});

    /**
     * @brief Handle health check request
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override;

    /**
     * @brief Get handler name
     * @return Handler name
     */
    std::string name() const override;

private:
    std::string serviceName_;
    std::vector<std::shared_ptr<IHealthCheck>> checks_;
};