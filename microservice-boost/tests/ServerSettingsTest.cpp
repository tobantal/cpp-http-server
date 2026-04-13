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
    EXPECT_EQ(settings.getPort(), 8080);

    unsetenv("SERVER_PORT");
}

TEST(ServerSettingsTest, InvalidPortEnvNoConfigFallsBackToDefault)
{
    setenv("SERVER_PORT", "abc", 1);
    unsetenv("SERVER_HOST");

    auto env = std::make_shared<Environment>();
    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "0.0.0.0");
    EXPECT_EQ(settings.getPort(), 8080);

    unsetenv("SERVER_PORT");
}