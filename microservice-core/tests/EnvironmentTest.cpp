#include <gtest/gtest.h>
#include "Environment.hpp"

class EnvironmentTest : public ::testing::Test {
protected:
    Environment env;
};

TEST_F(EnvironmentTest, SetAndGetPropertyInt) {
    env.setProperty("port", 8080);
    auto value = env.getProperty("port");
    ASSERT_TRUE(value.type() == typeid(int));
    EXPECT_EQ(std::any_cast<int>(value), 8080);
}

TEST_F(EnvironmentTest, SetAndGetPropertyString) {
    env.setProperty("host", std::string("localhost"));
    auto value = env.getProperty("host");
    ASSERT_TRUE(value.type() == typeid(std::string));
    EXPECT_EQ(std::any_cast<std::string>(value), "localhost");
}

TEST_F(EnvironmentTest, OverwriteProperty) {
    env.setProperty("key", 42);
    env.setProperty("key", 100);
    auto value = env.getProperty("key");
    EXPECT_EQ(std::any_cast<int>(value), 100);
}

TEST_F(EnvironmentTest, GetNonExistingPropertyThrows) {
    EXPECT_THROW({
        env.getProperty("missing_key");
    }, std::runtime_error);
}

TEST_F(EnvironmentTest, MultipleProperties) {
    env.setProperty("host", std::string("127.0.0.1"));
    env.setProperty("port", 3306);
    EXPECT_EQ(std::any_cast<std::string>(env.getProperty("host")), "127.0.0.1");
    EXPECT_EQ(std::any_cast<int>(env.getProperty("port")), 3306);
}
