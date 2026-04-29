#pragma once

#include "ports/input/IHttpHandler.hpp"

/**
 * @file HealthHandler.hpp
 * @brief Health check handler
 * @author Anton Tobolkin
 */

/**
 * @class HealthHandler
 * @brief Returns health status as JSON
 */
class HealthHandler : public IHttpHandler
{
public:
    /**
     * @brief Handle health check request
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override;
};