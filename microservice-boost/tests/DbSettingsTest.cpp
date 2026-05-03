#include <gtest/gtest.h>
#include "settings/DbSettings.hpp"
#include "adapters/secondary/Environment.hpp"

class DbSettingsTest : public ::testing::Test {
protected:
    void SetUp() override {
        unsetenv("AUTH_DB_HOST");
        unsetenv("AUTH_DB_PORT");
        unsetenv("AUTH_DB_NAME");
        unsetenv("AUTH_DB_USER");
        unsetenv("AUTH_DB_PASSWORD");
        unsetenv("AUTH_DB_POOL_MIN");
        unsetenv("AUTH_DB_POOL_MAX");
    }

    void TearDown() override {
        unsetenv("AUTH_DB_HOST");
        unsetenv("AUTH_DB_PORT");
        unsetenv("AUTH_DB_NAME");
        unsetenv("AUTH_DB_USER");
        unsetenv("AUTH_DB_PASSWORD");
        unsetenv("AUTH_DB_POOL_MIN");
        unsetenv("AUTH_DB_POOL_MAX");
    }
};

TEST_F(DbSettingsTest, DefaultValues_WhenNoEnvNoConfig) {
    DbSettings settings(nullptr, "AUTH");
    EXPECT_EQ(settings.getHost(), "AUTH-postgres");
    EXPECT_EQ(settings.getPort(), 5432);
    EXPECT_EQ(settings.getName(), "auth_db");
    EXPECT_EQ(settings.getUser(), "auth_user");
    EXPECT_EQ(settings.getPassword(), "auth_secret_password");
    EXPECT_EQ(settings.getMinConnections(), 2u);
    EXPECT_EQ(settings.getMaxConnections(), 10u);
}

TEST_F(DbSettingsTest, EnvOverridesDefaults) {
    setenv("AUTH_DB_HOST", "myhost", 1);
    setenv("AUTH_DB_PORT", "5433", 1);
    setenv("AUTH_DB_NAME", "mydb", 1);

    DbSettings settings(nullptr, "AUTH");
    EXPECT_EQ(settings.getHost(), "myhost");
    EXPECT_EQ(settings.getPort(), 5433);
    EXPECT_EQ(settings.getName(), "mydb");
    EXPECT_EQ(settings.getUser(), "auth_user");
}

TEST_F(DbSettingsTest, ConfigJsonOverridesDefaults_ButEnvOverridesConfig) {
    auto env = std::make_shared<Environment>();
    env->setProperty("auth.db.host", std::string("config-host"));
    env->setProperty("auth.db.port", 5555);

    setenv("AUTH_DB_HOST", "env-host", 1);

    DbSettings settings(env, "AUTH");
    EXPECT_EQ(settings.getHost(), "env-host");
    EXPECT_EQ(settings.getPort(), 5555);
}

TEST_F(DbSettingsTest, ConfigJsonOverridesDefaults_WhenNoEnv) {
    auto env = std::make_shared<Environment>();
    env->setProperty("auth.db.host", std::string("config-host"));
    env->setProperty("auth.db.port", 5555);

    DbSettings settings(env, "AUTH");
    EXPECT_EQ(settings.getHost(), "config-host");
    EXPECT_EQ(settings.getPort(), 5555);
}

TEST_F(DbSettingsTest, PoolSettings_EnvOverrides) {
    setenv("AUTH_DB_POOL_MIN", "5", 1);
    setenv("AUTH_DB_POOL_MAX", "20", 1);

    DbSettings settings(nullptr, "AUTH");
    EXPECT_EQ(settings.getMinConnections(), 5u);
    EXPECT_EQ(settings.getMaxConnections(), 20u);
}

TEST_F(DbSettingsTest, ConnectionString_ContainsAllParams) {
    DbSettings settings(nullptr, "AUTH");
    auto connStr = settings.getConnectionString();
    EXPECT_NE(connStr.find("host="), std::string::npos);
    EXPECT_NE(connStr.find("port="), std::string::npos);
    EXPECT_NE(connStr.find("dbname="), std::string::npos);
    EXPECT_NE(connStr.find("user="), std::string::npos);
    EXPECT_NE(connStr.find("password="), std::string::npos);
}

TEST_F(DbSettingsTest, DifferentPrefixes_GenerateDifferentDefaults) {
    DbSettings authSettings(nullptr, "AUTH");
    DbSettings brokerSettings(nullptr, "BROKER");

    EXPECT_EQ(authSettings.getHost(), "AUTH-postgres");
    EXPECT_EQ(brokerSettings.getHost(), "BROKER-postgres");
    EXPECT_NE(authSettings.getHost(), brokerSettings.getHost());
}

TEST_F(DbSettingsTest, ImplementsIDbSettings) {
    DbSettings settings(nullptr, "AUTH");
    IDbSettings& iface = settings;
    EXPECT_EQ(iface.getHost(), "AUTH-postgres");
    EXPECT_EQ(iface.getPort(), 5432);
    EXPECT_FALSE(iface.getConnectionString().empty());
}

TEST_F(DbSettingsTest, AllEnvVarsOverride) {
    setenv("AUTH_DB_HOST", "db.example.com", 1);
    setenv("AUTH_DB_PORT", "5434", 1);
    setenv("AUTH_DB_NAME", "production_db", 1);
    setenv("AUTH_DB_USER", "admin", 1);
    setenv("AUTH_DB_PASSWORD", "secret123", 1);
    setenv("AUTH_DB_POOL_MIN", "3", 1);
    setenv("AUTH_DB_POOL_MAX", "25", 1);

    DbSettings settings(nullptr, "AUTH");
    EXPECT_EQ(settings.getHost(), "db.example.com");
    EXPECT_EQ(settings.getPort(), 5434);
    EXPECT_EQ(settings.getName(), "production_db");
    EXPECT_EQ(settings.getUser(), "admin");
    EXPECT_EQ(settings.getPassword(), "secret123");
    EXPECT_EQ(settings.getMinConnections(), 3u);
    EXPECT_EQ(settings.getMaxConnections(), 25u);
}

TEST_F(DbSettingsTest, EmptyPrefix_UsesDirectConfigKeys) {
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("direct-host"));
    env->setProperty("db.port", 1234);

    DbSettings settings(env, "");
    EXPECT_EQ(settings.getHost(), "direct-host");
    EXPECT_EQ(settings.getPort(), 1234);
}
