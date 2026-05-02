#include <gtest/gtest.h>
#include "adapters/primary/PostgresHealthCheck.hpp"
#include "ports/output/IConnectionPool.hpp"
#include <memory>

class MockConnectionPool : public IConnectionPool
{
public:
    bool alive_ = true;
    size_t available_ = 5;
    size_t size_ = 10;

    std::string name() const override { return "MockConnectionPool"; }
    size_t available() const override { return available_; }
    size_t size() const override { return size_; }
    bool isAlive() const override { return alive_; }
    void shutdown(std::chrono::milliseconds) override {}
};

TEST(PostgresHealthCheckTest, HealthyPool)
{
    auto pool = std::make_shared<MockConnectionPool>();
    pool->alive_ = true;
    PostgresHealthCheck check(pool);

    auto status = check.check();
    EXPECT_EQ(status.name, "database");
    EXPECT_TRUE(status.healthy);
    EXPECT_NE(status.message.find("5/10"), std::string::npos);
}

TEST(PostgresHealthCheckTest, UnhealthyPool)
{
    auto pool = std::make_shared<MockConnectionPool>();
    pool->alive_ = false;
    pool->available_ = 0;
    PostgresHealthCheck check(pool);

    auto status = check.check();
    EXPECT_EQ(status.name, "database");
    EXPECT_FALSE(status.healthy);
    EXPECT_NE(status.message.find("0/10"), std::string::npos);
}
