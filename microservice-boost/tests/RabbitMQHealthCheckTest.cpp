#include <gtest/gtest.h>
#include "adapters/primary/RabbitMQHealthCheck.hpp"
#include "messaging/RabbitMQConnectionState.hpp"

TEST(RabbitMQHealthCheckTest, Connected)
{
    RabbitMQHealthCheck check([]()
                              { return RabbitMQConnectionState::Connected; });

    auto status = check.check();
    EXPECT_EQ(status.name, "rabbitmq");
    EXPECT_TRUE(status.healthy);
    EXPECT_EQ(status.message, "Connected");
}

TEST(RabbitMQHealthCheckTest, Idle)
{
    RabbitMQHealthCheck check([]()
                              { return RabbitMQConnectionState::Idle; });

    auto status = check.check();
    EXPECT_EQ(status.name, "rabbitmq");
    EXPECT_FALSE(status.healthy);
    EXPECT_EQ(status.message, "Idle");
}

TEST(RabbitMQHealthCheckTest, Reconnecting)
{
    RabbitMQHealthCheck check([]()
                              { return RabbitMQConnectionState::Reconnecting; });

    auto status = check.check();
    EXPECT_EQ(status.name, "rabbitmq");
    EXPECT_FALSE(status.healthy);
    EXPECT_EQ(status.message, "Reconnecting");
}

TEST(RabbitMQHealthCheckTest, Connecting)
{
    RabbitMQHealthCheck check([]()
                              { return RabbitMQConnectionState::Connecting; });

    auto status = check.check();
    EXPECT_EQ(status.name, "rabbitmq");
    EXPECT_FALSE(status.healthy);
    EXPECT_EQ(status.message, "Connecting");
}
