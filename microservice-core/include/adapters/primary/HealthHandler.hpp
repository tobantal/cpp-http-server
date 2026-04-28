#pragma once

#include "ports/input/IHttpHandler.hpp"
#include <string>

/**
 * @file HealthHandler.hpp
 * @brief Health check handler
 * @author Anton Tobolkin
 */

/**
 * @class HealthHandler
 * @brief Returns health status as JSON
 *
 * Returns HTTP 200 with body:
 *   {"status":"healthy","service":"<serviceName>"}
 *
 * If no service name is provided, defaults to "unknown".
 */
class HealthHandler : public IHttpHandler
{
public:
    /**
     * @brief Construct HealthHandler with service name
     * @param serviceName Name of the service for health response
     */
    explicit HealthHandler(const std::string& serviceName = "unknown")
        : serviceName_(serviceName) {}

    /**
     * @brief Handle health check request
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override
    {
        std::string body = R"({"status":"healthy","service":")" + serviceName_ + R"("})";
        res.setResult(200, "application/json", body);
    }

private:
    std::string serviceName_;
};
