#include "adapters/primary/HealthHandler.hpp"
#include "util/StringUtils.hpp"

HealthHandler::HealthHandler(const std::string& serviceName,
                             std::vector<std::shared_ptr<IHealthCheck>> checks)
    : serviceName_(serviceName), checks_(std::move(checks)) {}

std::string HealthHandler::name() const
{
    return "HealthHandler";
}

void HealthHandler::handle(IRequest &req, IResponse &res)
{
    (void)req;

    if (checks_.empty())
    {
        std::string body = R"({"status":"UP","service":")" + StringUtils::escapeJson(serviceName_) + R"("})";
        res.setResult(200, "application/json", body);
        return;
    }

    bool allHealthy = true;
    std::string checksJson;
    for (size_t i = 0; i < checks_.size(); ++i)
    {
        auto status = checks_[i]->check();
        if (!status.healthy)
        {
            allHealthy = false;
        }
        if (i > 0) checksJson += ",";
        checksJson += "\"" + StringUtils::escapeJson(status.name) + "\":{";
        checksJson += "\"status\":\"" + std::string(status.healthy ? "UP" : "DOWN") + "\"";
        if (!status.message.empty())
        {
            checksJson += ",\"message\":\"" + StringUtils::escapeJson(status.message) + "\"";
        }
        checksJson += "}";
    }

    std::string overallStatus = allHealthy ? "UP" : "DOWN";
    std::string body = R"({"status":")" + overallStatus + R"(","service":")" + StringUtils::escapeJson(serviceName_) + R"(","checks":{)" + checksJson + "}}";

    res.setResult(allHealthy ? 200 : 503, "application/json", body);
}