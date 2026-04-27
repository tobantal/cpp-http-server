#include <gtest/gtest.h>
#include "settings/DbSettings.hpp"

class DbSettingsTest : public ::testing::Test {
protected:
    void SetUp() override {
        unsetenv("TEST_DB_HOST");
        unsetenv("TEST_DB_PORT");
        unsetenv("TEST_DB_NAME");
        unsetenv("TEST_DB_USER");
        unsetenv("TEST_DB_PASSWORD");
        unsetenv("TEST_DB_POOL_MIN");
        unsetenv("TEST_DB_POOL_MAX");
    }

    void TearDown() override {
        unsetenv("TEST_DB_HOST");
        unsetenv("TEST_DB_PORT");
        unsetenv("TEST_DB_NAME");
        unsetenv("TEST_DB_USER");
        unsetenv("TEST_DB_PASSWORD");
        unsetenv("TEST_DB_POOL_MIN");
        unsetenv("TEST_DB_POOL_MAX");
    }
};

TEST_F(DbSettingsTest, DefaultValues_WhenNoEnvSet) {
    DbSettings settings("TEST");
    EXPECT_EQ(settings.getHost(), "TEST-postgres");
    EXPECT_EQ(settings.getPort(), 5432);
    EXPECT_EQ(settings.getName(), "test_db");
    EXPECT_EQ(settings.getUser(), "test_user");
    EXPECT_EQ(settings.getPassword(), "test_secret_password");
    EXPECT_EQ(settings.getMinConnections(), 2u);
    EXPECT_EQ(settings.getMaxConnections(), 10u);
}

TEST_F(DbSettingsTest, CustomDefaultHost) {
    DbSettings settings("TEST", "custom-host");
    EXPECT_EQ(settings.getHost(), "custom-host");
}

TEST_F(DbSettingsTest, EnvOverridesDefaults) {
    setenv("TEST_DB_HOST", "myhost", 1);
    setenv("TEST_DB_PORT", "5433", 1);
    setenv("TEST_DB_NAME", "mydb", 1);

    DbSettings settings("TEST");
    EXPECT_EQ(settings.getHost(), "myhost");
    EXPECT_EQ(settings.getPort(), 5433);
    EXPECT_EQ(settings.getName(), "mydb");
    EXPECT_EQ(settings.getUser(), "test_user");
}

TEST_F(DbSettingsTest, PoolSettings_EnvOverrides) {
    setenv("TEST_DB_POOL_MIN", "5", 1);
    setenv("TEST_DB_POOL_MAX", "20", 1);

    DbSettings settings("TEST");
    EXPECT_EQ(settings.getMinConnections(), 5u);
    EXPECT_EQ(settings.getMaxConnections(), 20u);
}

TEST_F(DbSettingsTest, ConnectionString_ContainsAllParams) {
    DbSettings settings("TEST");
    auto connStr = settings.getConnectionString();
    EXPECT_NE(connStr.find("host="), std::string::npos);
    EXPECT_NE(connStr.find("port="), std::string::npos);
    EXPECT_NE(connStr.find("dbname="), std::string::npos);
    EXPECT_NE(connStr.find("user="), std::string::npos);
    EXPECT_NE(connStr.find("password="), std::string::npos);
}

TEST_F(DbSettingsTest, DifferentPrefixes_GenerateDifferentDefaults) {
    DbSettings authSettings("AUTH");
    DbSettings brokerSettings("BROKER");

    EXPECT_EQ(authSettings.getHost(), "AUTH-postgres");
    EXPECT_EQ(brokerSettings.getHost(), "BROKER-postgres");
    EXPECT_NE(authSettings.getHost(), brokerSettings.getHost());
}

TEST_F(DbSettingsTest, ImplementsIDbSettings) {
    DbSettings settings("TEST");
    IDbSettings& iface = settings;
    EXPECT_EQ(iface.getHost(), "TEST-postgres");
    EXPECT_EQ(iface.getPort(), 5432);
    EXPECT_FALSE(iface.getConnectionString().empty());
}

TEST_F(DbSettingsTest, AllEnvVarsOverride) {
    setenv("TEST_DB_HOST", "db.example.com", 1);
    setenv("TEST_DB_PORT", "5434", 1);
    setenv("TEST_DB_NAME", "production_db", 1);
    setenv("TEST_DB_USER", "admin", 1);
    setenv("TEST_DB_PASSWORD", "secret123", 1);
    setenv("TEST_DB_POOL_MIN", "3", 1);
    setenv("TEST_DB_POOL_MAX", "25", 1);

    DbSettings settings("TEST");
    EXPECT_EQ(settings.getHost(), "db.example.com");
    EXPECT_EQ(settings.getPort(), 5434);
    EXPECT_EQ(settings.getName(), "production_db");
    EXPECT_EQ(settings.getUser(), "admin");
    EXPECT_EQ(settings.getPassword(), "secret123");
    EXPECT_EQ(settings.getMinConnections(), 3u);
    EXPECT_EQ(settings.getMaxConnections(), 25u);
}