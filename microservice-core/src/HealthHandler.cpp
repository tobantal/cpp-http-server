/**
 * @file HealthHandler.cpp
 * @brief HealthHandler implementation
 * @author Anton Tobolkin
 */

#include "adapters/primary/HealthHandler.hpp"

/**
 * @brief Handle health check - returns OK status
 * @param req HTTP request (unused)
 * @param res HTTP response with JSON status
 */
void HealthHandler::handle(IRequest &req, IResponse &res)
{
    res.setResult(200, "application/json", R"({"status": "ok"})");
}