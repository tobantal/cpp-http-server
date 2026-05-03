#include <gtest/gtest.h>
#include "application/HttpRetrySettings.hpp"
#include "adapters/secondary/Environment.hpp"

/**
 * @file HttpRetrySettingsTest.cpp
 * @brief Tests for HttpRetrySettings
 */

class HttpRetrySettingsTest : public ::testing::Test
{
protected:
    void TearDown() override {
        unsetEnv("HTTP_RETRY_STATUSES");
        unsetEnv("HTTP_RETRY_ON_NETWORK_ERROR");
    }

    void setEnv(const std::string& name, const std::string& value) {
        setenv(name.c_str(), value.c_str(), 1);
    }

    void unsetEnv(const std::string& name) {
        unsetenv(name.c_str());
    }
};

TEST_F(HttpRetrySettingsTest, DefaultRetryableStatuses)
{
    HttpRetrySettings settings(nullptr, "HTTP");
    const auto& statuses = settings.getRetryableStatuses();

    EXPECT_TRUE(statuses.count(500) > 0);
    EXPECT_TRUE(statuses.count(502) > 0);
    EXPECT_TRUE(statuses.count(503) > 0);
    EXPECT_TRUE(statuses.count(504) > 0);
    EXPECT_FALSE(statuses.count(400) > 0);
    EXPECT_FALSE(statuses.count(404) > 0);
}

TEST_F(HttpRetrySettingsTest, CustomRetryableStatuses)
{
    setEnv("HTTP_RETRY_STATUSES", "500,502,503");
    HttpRetrySettings settings(nullptr, "HTTP");
    const auto& statuses = settings.getRetryableStatuses();

    EXPECT_TRUE(statuses.count(500) > 0);
    EXPECT_TRUE(statuses.count(502) > 0);
    EXPECT_TRUE(statuses.count(503) > 0);
    EXPECT_FALSE(statuses.count(504) > 0);
}

TEST_F(HttpRetrySettingsTest, DefaultRetryOnNetworkError)
{
    HttpRetrySettings settings(nullptr, "HTTP");
    EXPECT_TRUE(settings.isRetryOnNetworkErrorEnabled());
}

TEST_F(HttpRetrySettingsTest, OverrideRetryOnNetworkError)
{
    setEnv("HTTP_RETRY_ON_NETWORK_ERROR", "false");
    HttpRetrySettings settings(nullptr, "HTTP");
    EXPECT_FALSE(settings.isRetryOnNetworkErrorEnabled());
}

TEST_F(HttpRetrySettingsTest, InheritsBaseSettings)
{
    setEnv("HTTP_RETRY_MAX_ATTEMPTS", "5");
    HttpRetrySettings settings(nullptr, "HTTP");

    EXPECT_EQ(settings.getMaxAttempts(), 5);
}

TEST_F(HttpRetrySettingsTest, EnvOverridesConfigJson)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("http.retry.maxAttempts", 10);

    setEnv("HTTP_RETRY_MAX_ATTEMPTS", "5");

    HttpRetrySettings settings(env, "HTTP");
    EXPECT_EQ(settings.getMaxAttempts(), 5);
}

TEST_F(HttpRetrySettingsTest, ConfigJsonOverridesDefaults_WhenNoEnv)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("http.retry.maxAttempts", 7);
    env->setProperty("http.retry.baseDelayMs", 2000);

    HttpRetrySettings settings(env, "HTTP");
    EXPECT_EQ(settings.getMaxAttempts(), 7);
    EXPECT_EQ(settings.getBaseDelay().count(), 2000);
}