#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "ports/output/IConnectionPool.hpp"
#include <memory>
#include <string>

/**
 * @file DatabaseHealthHandler.hpp
 * @brief Database health check handler
 * @author Anton Tobolkin
 */

/**
 * @class DatabaseHealthHandler
 * @brief HTTP handler that checks database connectivity
 *
 * Returns HTTP 200 with body:
 *   {"status":"healthy","database":"connected","pool":{"available":N,"size":M}}
 *
 * Or HTTP 503 with body:
 *   {"status":"unhealthy","database":"disconnected","pool":{"available":0,"size":0}}
 *
 * Depends on IConnectionPool for health check, available count, and size.
 */
class DatabaseHealthHandler : public IHttpHandler
{
public:
    /**
     * @brief Construct DatabaseHealthHandler
     * @param pool Connection pool to check
     */
    explicit DatabaseHealthHandler(std::shared_ptr<IConnectionPool> pool);

    void handle(IRequest &req, IResponse &res) override;

private:
    std::shared_ptr<IConnectionPool> pool_;
};