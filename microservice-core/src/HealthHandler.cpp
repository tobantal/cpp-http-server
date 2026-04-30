#include "adapters/primary/HealthHandler.hpp"

HealthHandler::HealthHandler(const std::string& serviceName)
    : serviceName_(serviceName) {}

void HealthHandler::handle(IRequest &req, IResponse &res) {
    std::string body = R"({"status":"healthy","service":")" + serviceName_ + R"("})";
    res.setResult(200, "application/json", body);
}