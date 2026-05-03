#include <gtest/gtest.h>
#include "adapters/primary/BoostBeastApplication.hpp"
#include "adapters/secondary/TestLogger.hpp"
#include <fstream>
#include <cstdio>

class TestApp : public BoostBeastApplication
{
public:
    explicit TestApp(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>())
        : BoostBeastApplication(logger) {}

    void configureInjection() override {}
    std::shared_ptr<IEnvironment> env() { return env_; }
};

class LoadEnvironmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        unsetenv("CONFIG_PATH");
    }

    void TearDown() override
    {
        unsetenv("CONFIG_PATH");
        std::remove("config.json");
        std::remove("/tmp/test_config.json");
    }

    void writeConfig(const std::string &path, const std::string &content)
    {
        std::ofstream f(path);
        f << content;
    }
};

TEST_F(LoadEnvironmentTest, DefaultConfigJsonInCurrentDir)
{
    writeConfig("config.json", R"({"server":{"port":9090}})");

    char arg0[] = {"app"};
    char *argv[] = {arg0, nullptr};
    TestApp app;
    app.loadEnvironment(1, argv);

    auto port = app.env()->get_optional<int>("server.port");
    ASSERT_TRUE(port.has_value());
    EXPECT_EQ(port.value(), 9090);
}

TEST_F(LoadEnvironmentTest, ConfigPathFromEnv)
{
    writeConfig("/tmp/test_config.json", R"({"server":{"host":"192.168.1.1"}})");
    setenv("CONFIG_PATH", "/tmp/test_config.json", 1);

    char arg0[] = {"app"};
    char *argv[] = {arg0, nullptr};
    TestApp app;
    app.loadEnvironment(1, argv);

    auto host = app.env()->get_optional<std::string>("server.host");
    ASSERT_TRUE(host.has_value());
    EXPECT_EQ(host.value(), "192.168.1.1");
}

TEST_F(LoadEnvironmentTest, CliConfigOverridesEnv)
{
    writeConfig("/tmp/test_config.json", R"({"server":{"port":7777}})");
    writeConfig("config.json", R"({"server":{"port":8888}})");
    setenv("CONFIG_PATH", "config.json", 1);

    char arg0[] = {"app"};
    char arg1[] = {"--config"};
    char arg2[] = {"/tmp/test_config.json"};
    char *argv[] = {arg0, arg1, arg2, nullptr};
    TestApp app;
    app.loadEnvironment(3, argv);

    auto port = app.env()->get_optional<int>("server.port");
    ASSERT_TRUE(port.has_value());
    EXPECT_EQ(port.value(), 7777);
}

TEST_F(LoadEnvironmentTest, ShortForm_C_Option)
{
    writeConfig("/tmp/test_config.json", R"({"server":{"port":5555}})");

    char arg0[] = {"app"};
    char arg1[] = {"-c"};
    char arg2[] = {"/tmp/test_config.json"};
    char *argv[] = {arg0, arg1, arg2, nullptr};
    TestApp app;
    app.loadEnvironment(3, argv);

    auto port = app.env()->get_optional<int>("server.port");
    ASSERT_TRUE(port.has_value());
    EXPECT_EQ(port.value(), 5555);
}

TEST_F(LoadEnvironmentTest, MissingConfigFileNoError)
{
    std::remove("config.json");

    char arg0[] = {"app"};
    char *argv[] = {arg0, nullptr};
    TestApp app;
    EXPECT_NO_THROW(app.loadEnvironment(1, argv));
}

TEST_F(LoadEnvironmentTest, ConfigPathEnvFallbackWhenNoCli)
{
    writeConfig("/tmp/test_config.json", R"({"server":{"port":4444}})");
    setenv("CONFIG_PATH", "/tmp/test_config.json", 1);

    char arg0[] = {"app"};
    char *argv[] = {arg0, nullptr};
    TestApp app;
    app.loadEnvironment(1, argv);

    auto port = app.env()->get_optional<int>("server.port");
    ASSERT_TRUE(port.has_value());
    EXPECT_EQ(port.value(), 4444);
}