#include <gtest/gtest.h>
#include "application/CircuitBreakerSettings.hpp"

class CircuitBreakerSettingsTest : public ::testing::Test
{
protected:
    void TearDown() override
    {
        unsetEnv("HTTP_CB_FAILURE_THRESHOLD");
        unsetEnv("HTTP_CB_RESET_TIMEOUT_MS");
        unsetEnv("HTTP_CB_HALF_OPEN_MAX_CALLS");
    }

    void setEnv(const std::string& name, const std::string& value)
    {
        setenv(name.c_str(), value.c_str(), 1);
    }

    void unsetEnv(const std::string& name)
    {
        unsetenv(name.c_str());
    }
};

TEST_F(CircuitBreakerSettingsTest, DefaultValues)
{
    CircuitBreakerSettings settings("HTTP");
    EXPECT_EQ(settings.getFailureThreshold(), 5);
    EXPECT_EQ(settings.getResetTimeout(), std::chrono::milliseconds(30000));
    EXPECT_EQ(settings.getHalfOpenMaxCalls(), 3);
}

TEST_F(CircuitBreakerSettingsTest, CustomFailureThreshold)
{
    setEnv("HTTP_CB_FAILURE_THRESHOLD", "10");
    CircuitBreakerSettings settings("HTTP");
    EXPECT_EQ(settings.getFailureThreshold(), 10);
}

TEST_F(CircuitBreakerSettingsTest, CustomResetTimeout)
{
    setEnv("HTTP_CB_RESET_TIMEOUT_MS", "60000");
    CircuitBreakerSettings settings("HTTP");
    EXPECT_EQ(settings.getResetTimeout(), std::chrono::milliseconds(60000));
}

TEST_F(CircuitBreakerSettingsTest, CustomHalfOpenMaxCalls)
{
    setEnv("HTTP_CB_HALF_OPEN_MAX_CALLS", "5");
    CircuitBreakerSettings settings("HTTP");
    EXPECT_EQ(settings.getHalfOpenMaxCalls(), 5);
}

TEST_F(CircuitBreakerSettingsTest, DifferentPrefix)
{
    setEnv("SERVICE_CB_FAILURE_THRESHOLD", "7");
    CircuitBreakerSettings settings("SERVICE");
    EXPECT_EQ(settings.getFailureThreshold(), 7);
}