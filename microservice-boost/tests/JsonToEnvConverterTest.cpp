#include <gtest/gtest.h>
#include "adapters/primary/JsonToEnvConverter.hpp"
#include "ports/output/IEnvironment.hpp"
#include "domain/error/ConvertError.hpp"

TEST(JsonToEnvConverterTest, ParseValidJson)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"name": "admin", "age": 30})");

    EXPECT_EQ(env->get<std::string>("name"), "admin");
    EXPECT_EQ(env->get<int>("age"), 30);
}

TEST(JsonToEnvConverterTest, ParseStringNumber)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"price": 99.99})");

    EXPECT_EQ(env->get<double>("price"), 99.99);
}

TEST(JsonToEnvConverterTest, ParseBoolean)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"active": true, "deleted": false})");

    EXPECT_EQ(env->get<bool>("active"), true);
    EXPECT_EQ(env->get<bool>("deleted"), false);
}

TEST(JsonToEnvConverterTest, ParseNullGetThrowsConvertError)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"value": null})");

    EXPECT_THROW(env->get<std::string>("value"), ConvertError);
}

TEST(JsonToEnvConverterTest, ParseNullOptionalReturnsNullopt)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"value": null})");

    auto result = env->get_optional<std::string>("value");
    EXPECT_EQ(result, std::nullopt);
}

TEST(JsonToEnvConverterTest, ParseNestedObject)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"data": {"key": "value"}})");

    EXPECT_TRUE(env->getProperty("data").has_value());
}

TEST(JsonToEnvConverterTest, ParseEmptyObjectGetOptionalReturnsDefault)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({})");

    EXPECT_EQ(env->get_optional<std::string>("any").value_or("default"), "default");
}

TEST(JsonToEnvConverterTest, ParseInvalidJsonThrows)
{
    JsonToEnvConverter converter;
    EXPECT_THROW(converter.convert("not json"), ConvertError);
}

TEST(JsonToEnvConverterTest, ParseEmptyStringThrows)
{
    JsonToEnvConverter converter;
    EXPECT_THROW(converter.convert(""), ConvertError);
}

TEST(JsonToEnvConverterTest, MissingPropertyGetOptionalReturnsDefault)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({"name": "admin"})");

    EXPECT_EQ(env->get_optional<std::string>("missing").value_or("default"), "default");
}

TEST(JsonToEnvConverterTest, MultipleFields)
{
    JsonToEnvConverter converter;
    auto env = converter.convert(R"({
        "username": "admin",
        "level": 5,
        "balance": 100.50,
        "active": true
    })");

    EXPECT_EQ(env->get<std::string>("username"), "admin");
    EXPECT_EQ(env->get<int>("level"), 5);
    EXPECT_EQ(env->get<double>("balance"), 100.50);
    EXPECT_EQ(env->get<bool>("active"), true);
}