#include <gtest/gtest.h>
#include "application/RetrySettings.hpp"
#include "adapters/secondary/Environment.hpp"

/**
 * @file RetrySettingsTest.cpp
 * @brief Tests for RetrySettings
 */

class RetrySettingsTest : public ::testing::Test
{
protected:
    void TearDown() override {
        unsetEnv("TEST_RETRY_MAX_ATTEMPTS");
        unsetEnv("TEST_RETRY_BASE_DELAY_MS");
        unsetEnv("TEST_RETRY_MULTIPLIER");
        unsetEnv("TEST_RETRY_MAX_DELAY_MS");
    }

    void setEnv(const std::string& name, const std::string& value) {
        setenv(name.c_str(), value.c_str(), 1);
    }

    void unsetEnv(const std::string& name) {
        unsetenv(name.c_str());
    }
};

TEST_F(RetrySettingsTest, DefaultValues)
{
    RetrySettings settings(nullptr, "TEST");
    EXPECT_EQ(settings.getMaxAttempts(), 3);
    EXPECT_EQ(settings.getBaseDelay().count(), 1000);
    EXPECT_EQ(settings.getMultiplier(), 2.0);
    EXPECT_EQ(settings.getMaxDelay().count(), 30000);
}

TEST_F(RetrySettingsTest, OverrideMaxAttempts)
{
    setEnv("TEST_RETRY_MAX_ATTEMPTS", "5");
    RetrySettings settings(nullptr, "TEST");
    EXPECT_EQ(settings.getMaxAttempts(), 5);
}

TEST_F(RetrySettingsTest, OverrideBaseDelay)
{
    setEnv("TEST_RETRY_BASE_DELAY_MS", "500");
    RetrySettings settings(nullptr, "TEST");
    EXPECT_EQ(settings.getBaseDelay().count(), 500);
}

TEST_F(RetrySettingsTest, OverrideMultiplier)
{
    setEnv("TEST_RETRY_MULTIPLIER", "1.5");
    RetrySettings settings(nullptr, "TEST");
    EXPECT_EQ(settings.getMultiplier(), 1.5);
}

TEST_F(RetrySettingsTest, OverrideMaxDelay)
{
    setEnv("TEST_RETRY_MAX_DELAY_MS", "60000");
    RetrySettings settings(nullptr, "TEST");
    EXPECT_EQ(settings.getMaxDelay().count(), 60000);
}

TEST_F(RetrySettingsTest, EnvOverridesConfigJson)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("test.retry.maxAttempts", 10);

    setEnv("TEST_RETRY_MAX_ATTEMPTS", "5");

    RetrySettings settings(env, "TEST");
    EXPECT_EQ(settings.getMaxAttempts(), 5);
}