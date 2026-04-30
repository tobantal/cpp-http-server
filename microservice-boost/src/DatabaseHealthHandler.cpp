#include "adapters/primary/DatabaseHealthHandler.hpp"
#include "domain/HttpStatus.hpp"
#include "util/StringUtils.hpp"

DatabaseHealthHandler::DatabaseHealthHandler(std::shared_ptr<IConnectionPool> pool)
    : pool_(std::move(pool))
{
}

void DatabaseHealthHandler::handle(IRequest &req, IResponse &res)
{
    (void)req;

    bool alive = pool_->isAlive();
    size_t available = pool_->available();
    size_t size = pool_->size();

    std::string status = alive ? "healthy" : "unhealthy";
    std::string dbStatus = alive ? "connected" : "disconnected";

    std::string body = R"({"status":")" + status + R"(","database":")" + dbStatus +
        R"(","pool":{"available":)" + std::to_string(available) +
        R"(,"size":)" + std::to_string(size) + "}}";

    res.setResult(alive ? HttpStatus::Ok : HttpStatus::ServiceUnavailable,
        "application/json", body);
}