#include <gtest/gtest.h>
#include "adapters/secondary/Environment.hpp"
#include "domain/error/ConvertError.hpp"
#include <optional>
#include <string>

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

TEST_F(EnvironmentTest, GetNonExistingPropertyThrowsConvertError) {
    EXPECT_THROW({
        env.getProperty("missing_key");
    }, ConvertError);
}

TEST_F(EnvironmentTest, MultipleProperties) {
    env.setProperty("host", std::string("127.0.0.1"));
    env.setProperty("port", 3306);
    EXPECT_EQ(std::any_cast<std::string>(env.getProperty("host")), "127.0.0.1");
    EXPECT_EQ(std::any_cast<int>(env.getProperty("port")), 3306);
}

// --- get<T>(key) tests ---

TEST_F(EnvironmentTest, GetTypedValue) {
    env.setProperty("port", 8080);
    EXPECT_EQ(env.get<int>("port"), 8080);
}

TEST_F(EnvironmentTest, GetTypedString) {
    env.setProperty("host", std::string("localhost"));
    EXPECT_EQ(env.get<std::string>("host"), "localhost");
}

TEST_F(EnvironmentTest, GetMissingKeyThrowsConvertError) {
    EXPECT_THROW(env.get<int>("missing"), ConvertError);
}

TEST_F(EnvironmentTest, GetTypeMismatchThrowsConvertError) {
    env.setProperty("port", 8080);
    EXPECT_THROW(env.get<std::string>("port"), ConvertError);
}

TEST_F(EnvironmentTest, GetDoubleFromIntThrowsConvertError) {
    env.setProperty("count", 42);
    EXPECT_THROW(env.get<double>("count"), ConvertError);
}

// --- get_optional<T>(key) tests ---

TEST_F(EnvironmentTest, GetOptionalReturnsValue) {
    env.setProperty("port", 8080);
    auto result = env.get_optional<int>("port");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 8080);
}

TEST_F(EnvironmentTest, GetOptionalMissingKeyReturnsNullopt) {
    auto result = env.get_optional<int>("missing");
    EXPECT_EQ(result, std::nullopt);
}

TEST_F(EnvironmentTest, GetOptionalTypeMismatchThrowsConvertError) {
    env.setProperty("port", 8080);
    EXPECT_THROW(env.get_optional<std::string>("port"), ConvertError);
}

TEST_F(EnvironmentTest, GetOptionalString) {
    env.setProperty("name", std::string("admin"));
    auto result = env.get_optional<std::string>("name");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "admin");
}

TEST_F(EnvironmentTest, GetOptionalDouble) {
    env.setProperty("price", 99.99);
    auto result = env.get_optional<double>("price");
    ASSERT_TRUE(result.has_value());
    EXPECT_DOUBLE_EQ(result.value(), 99.99);
}

TEST_F(EnvironmentTest, GetOptionalBool) {
    env.setProperty("active", true);
    auto result = env.get_optional<bool>("active");
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), true);
}

// --- hasProperty tests ---

TEST_F(EnvironmentTest, HasPropertyReturnsTrue) {
    env.setProperty("port", 8080);
    EXPECT_TRUE(env.hasProperty("port"));
}

TEST_F(EnvironmentTest, HasPropertyReturnsFalse) {
    EXPECT_FALSE(env.hasProperty("missing"));
}

TEST_F(EnvironmentTest, HasPropertyAfterOverwrite) {
    env.setProperty("key", 1);
    env.setProperty("key", 2);
    EXPECT_TRUE(env.hasProperty("key"));
}