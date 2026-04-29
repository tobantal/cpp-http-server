#include "adapters/primary/HealthHandler.hpp"

void HealthHandler::handle(IRequest &req, IResponse &res)
{
    res.setResult(200, "application/json", R"({"status": "ok"})");
}