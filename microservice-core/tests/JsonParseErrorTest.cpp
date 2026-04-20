#include <gtest/gtest.h>
#include "domain/error/JsonParseError.hpp"

TEST(JsonParseErrorTest, DefaultMessage)
{
    JsonParseError e;
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Invalid JSON");
}

TEST(JsonParseErrorTest, CustomMessage)
{
    JsonParseError e("Unexpected token at position 5");
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Unexpected token at position 5");
}

TEST(JsonParseErrorTest, IsHttpError)
{
    try
    {
        throw JsonParseError("bad json");
    }
    catch (const HttpError &e)
    {
        EXPECT_EQ(e.statusCode(), 400);
        EXPECT_EQ(e.message(), "bad json");
    }
}
