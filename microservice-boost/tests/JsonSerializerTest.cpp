#include <gtest/gtest.h>
#include "JsonSerializer.hpp"
#include "JsonParseError.hpp"
#include <string>

struct UserDto
{
    std::string name;
    int age;

    bool operator==(const UserDto &other) const
    {
        return name == other.name && age == other.age;
    }
};

void to_json(nlohmann::json &j, const UserDto &u)
{
    j = nlohmann::json{{"name", u.name}, {"age", u.age}};
}

void from_json(const nlohmann::json &j, UserDto &u)
{
    u.name = j.at("name").get<std::string>();
    u.age = j.at("age").get<int>();
}

TEST(JsonSerializerTest, SerializeDto)
{
    UserDto user{"Alice", 30};
    std::string json = serialize(user);

    EXPECT_EQ(json, R"({"age":30,"name":"Alice"})");
}

TEST(JsonSerializerTest, DeserializeDto)
{
    std::string json = R"({"name":"Bob","age":25})";
    UserDto user = deserialize<UserDto>(json);

    EXPECT_EQ(user.name, "Bob");
    EXPECT_EQ(user.age, 25);
}

TEST(JsonSerializerTest, DeserializeInvalidJson)
{
    std::string json = "{invalid json}";
    EXPECT_THROW(deserialize<UserDto>(json), JsonParseError);
}

TEST(JsonSerializerTest, DeserializeMissingField)
{
    std::string json = R"({"name":"Charlie"})";
    EXPECT_THROW(deserialize<UserDto>(json), JsonParseError);
}

TEST(JsonSerializerTest, DeserializeTypeError)
{
    std::string json = R"({"name":"Dave","age":"not_a_number"})";
    EXPECT_THROW(deserialize<UserDto>(json), JsonParseError);
}

TEST(JsonSerializerTest, RoundTrip)
{
    UserDto original{"Eve", 28};
    std::string json = serialize(original);
    UserDto restored = deserialize<UserDto>(json);

    EXPECT_EQ(original, restored);
}

TEST(JsonSerializerTest, JsonParseErrorStatusCode)
{
    try
    {
        throw JsonParseError("test error");
    }
    catch (const HttpError &e)
    {
        EXPECT_EQ(e.statusCode(), 400);
    }
}