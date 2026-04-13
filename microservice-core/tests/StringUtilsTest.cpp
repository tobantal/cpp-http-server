#include <gtest/gtest.h>
#include "StringUtils.hpp"
#include "PathParamExtractor.hpp"

TEST(StringUtilsTest, ToLowerEmpty)
{
    EXPECT_EQ(StringUtils::toLower(""), "");
}

TEST(StringUtilsTest, ToLowerLowercase)
{
    EXPECT_EQ(StringUtils::toLower("hello"), "hello");
}

TEST(StringUtilsTest, ToLowerUppercase)
{
    EXPECT_EQ(StringUtils::toLower("HELLO"), "hello");
}

TEST(StringUtilsTest, ToLowerMixed)
{
    EXPECT_EQ(StringUtils::toLower("Content-Type"), "content-type");
}

TEST(StringUtilsTest, ToLowerWithNumbers)
{
    EXPECT_EQ(StringUtils::toLower("HTTP1.1"), "http1.1");
}

TEST(StringUtilsTest, SplitPathEmpty)
{
    auto result = StringUtils::splitPath("");
    EXPECT_TRUE(result.empty());
}

TEST(StringUtilsTest, SplitPathRoot)
{
    auto result = StringUtils::splitPath("/");
    EXPECT_TRUE(result.empty());
}

TEST(StringUtilsTest, SplitPathSingleSegment)
{
    auto result = StringUtils::splitPath("/api");
    ASSERT_EQ(result.size(), 1u);
    EXPECT_EQ(result[0], "api");
}

TEST(StringUtilsTest, SplitPathMultipleSegments)
{
    auto result = StringUtils::splitPath("/api/v1/users");
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0], "api");
    EXPECT_EQ(result[1], "v1");
    EXPECT_EQ(result[2], "users");
}

TEST(StringUtilsTest, SplitPathTrailingSlash)
{
    auto result = StringUtils::splitPath("/api/v1/");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "api");
    EXPECT_EQ(result[1], "v1");
}

TEST(StringUtilsTest, SplitPathDoubleSlash)
{
    auto result = StringUtils::splitPath("/api//users");
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], "api");
    EXPECT_EQ(result[1], "users");
}

TEST(PathParamExtractorTest, EmptyPattern)
{
    auto result = PathParamExtractor::getByIndex("/api/v1/orders/123", "", 0);
    EXPECT_FALSE(result.has_value());
}

TEST(PathParamExtractorTest, NoWildcards)
{
    auto result = PathParamExtractor::getByIndex("/api/v1/orders", "/api/v1/orders", 0);
    EXPECT_FALSE(result.has_value());
}

TEST(PathParamExtractorTest, SingleWildcard)
{
    auto result = PathParamExtractor::getByIndex("/api/v1/orders/123", "/api/v1/orders/*", 0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), "123");
}

TEST(PathParamExtractorTest, MultipleWildcards)
{
    auto result0 = PathParamExtractor::getByIndex("/api/v1/ord-123/item-456", "/api/v1/*/*", 0);
    ASSERT_TRUE(result0.has_value());
    EXPECT_EQ(result0.value(), "ord-123");

    auto result1 = PathParamExtractor::getByIndex("/api/v1/ord-123/item-456", "/api/v1/*/*", 1);
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1.value(), "item-456");
}

TEST(PathParamExtractorTest, WildcardIndexOutOfRange)
{
    auto result = PathParamExtractor::getByIndex("/api/v1/orders/123", "/api/v1/orders/*", 1);
    EXPECT_FALSE(result.has_value());
}