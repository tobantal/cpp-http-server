#include <gtest/gtest.h>
#include <memory>
#include <cstdlib>

#include "settings/ServerSettings.hpp"
#include "Environment.hpp"

TEST(ServerSettingsTest, LoadFromEnvironment)
{
    setenv("SERVER_HOST", "192.168.1.1", 1);
    setenv("SERVER_PORT", "9090", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "192.168.1.1");
    EXPECT_EQ(settings.getPort(), 9090);

    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
}

TEST(ServerSettingsTest, EnvOverridesConfig)
{
    setenv("SERVER_HOST", "10.0.0.1", 1);
    setenv("SERVER_PORT", "3000", 1);

    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("127.0.0.1"));
    env->setProperty("server.port", 8080);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "10.0.0.1");
    EXPECT_EQ(settings.getPort(), 3000);

    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
}

TEST(ServerSettingsTest, FallbackToConfigWhenNoEnv)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("127.0.0.1"));
    env->setProperty("server.port", 8080);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "127.0.0.1");
    EXPECT_EQ(settings.getPort(), 8080);
}

TEST(ServerSettingsTest, DefaultsWhenNoEnvNoConfig)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "0.0.0.0");
    EXPECT_EQ(settings.getPort(), 8080);
}

TEST(ServerSettingsTest, HostFromEnvPortFromConfig)
{
    setenv("SERVER_HOST", "10.0.0.2", 1);
    unsetenv("SERVER_PORT");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.port", 7070);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "10.0.0.2");
    EXPECT_EQ(settings.getPort(), 7070);

    unsetenv("SERVER_HOST");
}

TEST(ServerSettingsTest, InvalidPortEnvFallsBackToConfig)
{
    setenv("SERVER_PORT", "not_a_number", 1);
    unsetenv("SERVER_HOST");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("localhost"));
    env->setProperty("server.port", 8080);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "localhost");
    unsetenv("SERVER_PORT");
}

TEST(ServerSettingsTest, DefaultMaxRequestBodySize)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_MAX_REQUEST_BODY_SIZE");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxRequestBodySize(), 1048576);
}

TEST(ServerSettingsTest, MaxRequestBodySizeFromEnv)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    setenv("SERVER_MAX_REQUEST_BODY_SIZE", "2097152", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxRequestBodySize(), 2097152);

    unsetenv("SERVER_MAX_REQUEST_BODY_SIZE");
}

TEST(ServerSettingsTest, MaxRequestBodySizeFromConfig)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_MAX_REQUEST_BODY_SIZE");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.maxRequestBodySize", static_cast<size_t>(524288));

    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxRequestBodySize(), 524288);
}

TEST(ServerSettingsTest, MaxRequestBodySizeEnvOverridesConfig)
{
    setenv("SERVER_MAX_REQUEST_BODY_SIZE", "4194304", 1);

    auto env = std::make_shared<Environment>();
    env->setProperty("server.maxRequestBodySize", static_cast<size_t>(524288));

    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxRequestBodySize(), 4194304);

    unsetenv("SERVER_MAX_REQUEST_BODY_SIZE");
}

TEST(ServerSettingsTest, InvalidMaxRequestBodySizeEnvUsesDefault)
{
    setenv("SERVER_MAX_REQUEST_BODY_SIZE", "not_a_number", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxRequestBodySize(), 1048576);

    unsetenv("SERVER_MAX_REQUEST_BODY_SIZE");
}

TEST(ServerSettingsTest, DefaultReadTimeout)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_READ_TIMEOUT_MS");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getReadTimeout(), std::chrono::milliseconds(30000));
}

TEST(ServerSettingsTest, ReadTimeoutFromEnv)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    setenv("SERVER_READ_TIMEOUT_MS", "5000", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getReadTimeout(), std::chrono::milliseconds(5000));

    unsetenv("SERVER_READ_TIMEOUT_MS");
}

TEST(ServerSettingsTest, ReadTimeoutFromConfig)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_READ_TIMEOUT_MS");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.readTimeoutMs", 10000);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getReadTimeout(), std::chrono::milliseconds(10000));
}

TEST(ServerSettingsTest, DefaultWriteTimeout)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_WRITE_TIMEOUT_MS");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getWriteTimeout(), std::chrono::milliseconds(30000));
}

TEST(ServerSettingsTest, WriteTimeoutFromEnv)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    setenv("SERVER_WRITE_TIMEOUT_MS", "5000", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getWriteTimeout(), std::chrono::milliseconds(5000));

    unsetenv("SERVER_WRITE_TIMEOUT_MS");
}

TEST(ServerSettingsTest, InvalidReadTimeoutEnvUsesDefault)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    setenv("SERVER_READ_TIMEOUT_MS", "not_a_number", 1);
    unsetenv("SERVER_WRITE_TIMEOUT_MS");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getReadTimeout(), std::chrono::milliseconds(30000));

    unsetenv("SERVER_READ_TIMEOUT_MS");
}

TEST(ServerSettingsTest, DefaultMaxConnectionsIsUnlimited)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_MAX_CONNECTIONS");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxConnections(), 0u);
}

TEST(ServerSettingsTest, MaxConnectionsFromEnv)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    setenv("SERVER_MAX_CONNECTIONS", "100", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxConnections(), 100u);

    unsetenv("SERVER_MAX_CONNECTIONS");
}

TEST(ServerSettingsTest, MaxConnectionsFromConfig)
{
    unsetenv("SERVER_HOST");
    unsetenv("SERVER_PORT");
    unsetenv("SERVER_MAX_CONNECTIONS");

    auto env = std::make_shared<Environment>();
    env->setProperty("server.maxConnections", static_cast<size_t>(50));

    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxConnections(), 50u);
}

TEST(ServerSettingsTest, MaxConnectionsEnvOverridesConfig)
{
    setenv("SERVER_MAX_CONNECTIONS", "200", 1);

    auto env = std::make_shared<Environment>();
    env->setProperty("server.maxConnections", static_cast<size_t>(50));

    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxConnections(), 200u);

    unsetenv("SERVER_MAX_CONNECTIONS");
}

TEST(ServerSettingsTest, InvalidMaxConnectionsEnvUsesDefault)
{
    setenv("SERVER_MAX_CONNECTIONS", "not_a_number", 1);

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getMaxConnections(), 0u);

    unsetenv("SERVER_MAX_CONNECTIONS");
}