#include <gtest/gtest.h>
#include "adapters/primary/HealthHandler.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"

namespace
{
    class HealthyCheck : public IHealthCheck
    {
    public:
        explicit HealthyCheck(const std::string &name) : name_(name) {}
        HealthStatus check() const override
        {
            return {name_, true, "OK"};
        }

    private:
        std::string name_;
    };

    class UnhealthyCheck : public IHealthCheck
    {
    public:
        explicit UnhealthyCheck(const std::string &name) : name_(name) {}
        HealthStatus check() const override
        {
            return {name_, false, "Connection refused"};
        }

    private:
        std::string name_;
    };
}

TEST(HealthHandlerTest, NoChecks_ReturnsUp)
{
    HealthHandler handler("test-service");
    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_NE(res.getBody().find("\"status\":\"UP\""), std::string::npos);
    EXPECT_NE(res.getBody().find("\"service\":\"test-service\""), std::string::npos);
}

TEST(HealthHandlerTest, AllHealthy_ReturnsUp)
{
    std::vector<std::shared_ptr<IHealthCheck>> checks;
    checks.push_back(std::make_shared<HealthyCheck>("database"));
    checks.push_back(std::make_shared<HealthyCheck>("rabbitmq"));

    HealthHandler handler("my-service", std::move(checks));
    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_NE(res.getBody().find("\"status\":\"UP\""), std::string::npos);
    EXPECT_NE(res.getBody().find("\"database\""), std::string::npos);
    EXPECT_NE(res.getBody().find("\"rabbitmq\""), std::string::npos);
}

TEST(HealthHandlerTest, OneUnhealthy_ReturnsDown503)
{
    std::vector<std::shared_ptr<IHealthCheck>> checks;
    checks.push_back(std::make_shared<HealthyCheck>("database"));
    checks.push_back(std::make_shared<UnhealthyCheck>("rabbitmq"));

    HealthHandler handler("my-service", std::move(checks));
    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    EXPECT_EQ(res.getStatus(), 503);
    EXPECT_NE(res.getBody().find("\"status\":\"DOWN\""), std::string::npos);
    EXPECT_NE(res.getBody().find("\"database\":{\"status\":\"UP\""), std::string::npos);
    EXPECT_NE(res.getBody().find("\"rabbitmq\":{\"status\":\"DOWN\""), std::string::npos);
    EXPECT_NE(res.getBody().find("Connection refused"), std::string::npos);
}

TEST(HealthHandlerTest, AllUnhealthy_ReturnsDown503)
{
    std::vector<std::shared_ptr<IHealthCheck>> checks;
    checks.push_back(std::make_shared<UnhealthyCheck>("database"));

    HealthHandler handler("my-service", std::move(checks));
    SimpleRequest req("GET", "/health", "", "127.0.0.1", 80);
    SimpleResponse res;
    handler.handle(req, res);

    EXPECT_EQ(res.getStatus(), 503);
    EXPECT_NE(res.getBody().find("\"status\":\"DOWN\""), std::string::npos);
}

TEST(HealthHandlerTest, Name_ReturnsHealthHandler)
{
    HealthHandler handler("svc");
    EXPECT_EQ(handler.name(), "HealthHandler");
}
