#include <gtest/gtest.h>
#include "StringUtils.hpp"
#include "PathParamExtractor.hpp"
#include <set>

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

// --- StringUtils::escapeJson ---

TEST(StringUtilsTest, EscapeJson_PlainString)
{
    EXPECT_EQ(StringUtils::escapeJson("hello"), "hello");
}

TEST(StringUtilsTest, EscapeJson_EmptyString)
{
    EXPECT_EQ(StringUtils::escapeJson(""), "");
}

TEST(StringUtilsTest, EscapeJson_DoubleQuote)
{
    EXPECT_EQ(StringUtils::escapeJson(R"(he said "hello")"), R"(he said \"hello\")");
}

TEST(StringUtilsTest, EscapeJson_Backslash)
{
    EXPECT_EQ(StringUtils::escapeJson(R"(path\to\file)"), R"(path\\to\\file)");
}

TEST(StringUtilsTest, EscapeJson_Newline)
{
    EXPECT_EQ(StringUtils::escapeJson("line1\nline2"), "line1\\nline2");
}

TEST(StringUtilsTest, EscapeJson_CarriageReturn)
{
    EXPECT_EQ(StringUtils::escapeJson("line1\rline2"), "line1\\rline2");
}

TEST(StringUtilsTest, EscapeJson_Tab)
{
    EXPECT_EQ(StringUtils::escapeJson("col1\tcol2"), "col1\\tcol2");
}

TEST(StringUtilsTest, EscapeJson_ControlCharacter)
{
    std::string input = "text";
    input += '\x01';
    input += "char";
    std::string result = StringUtils::escapeJson(input);
    EXPECT_EQ(result, "text\\u0001char");
}

TEST(StringUtilsTest, EscapeJson_MultipleSpecialChars)
{
    EXPECT_EQ(StringUtils::escapeJson(R"("path"\n)"), R"(\"path\"\\n)");
}

TEST(StringUtilsTest, EscapeJson_JsonInjectionAttack)
{
    std::string malicious = R"(ok","injected":"true)";
    std::string expected = R"(ok\",\"injected\":\"true)";
    EXPECT_EQ(StringUtils::escapeJson(malicious), expected);
}

// --- StringUtils::urlDecode ---

TEST(StringUtilsTest, UrlDecode_PlainString)
{
    EXPECT_EQ(StringUtils::urlDecode("/api/v1/orders"), "/api/v1/orders");
}

TEST(StringUtilsTest, UrlDecode_EncodedSpaces)
{
    EXPECT_EQ(StringUtils::urlDecode("/api/v1/my%20order"), "/api/v1/my order");
}

TEST(StringUtilsTest, UrlDecode_EncodedUnicode)
{
    EXPECT_EQ(StringUtils::urlDecode("/%C3%BCnicode"), "/ünicode");
}

TEST(StringUtilsTest, UrlDecode_MultipleEncodedChars)
{
    EXPECT_EQ(StringUtils::urlDecode("/api/hello%20world%21"), "/api/hello world!");
}

TEST(StringUtilsTest, UrlDecode_PlusAsSpace)
{
    EXPECT_EQ(StringUtils::urlDecode("hello+world"), "hello world");
}

TEST(StringUtilsTest, UrlDecode_IncompletePercent)
{
    EXPECT_EQ(StringUtils::urlDecode("%2"), "%2");
}

TEST(StringUtilsTest, UrlDecode_InvalidHex)
{
    EXPECT_EQ(StringUtils::urlDecode("%ZZ"), "%ZZ");
}

TEST(StringUtilsTest, UrlDecode_EmptyString)
{
    EXPECT_EQ(StringUtils::urlDecode(""), "");
}

// --- StringUtils::generateUuid ---

TEST(StringUtilsTest, GenerateUuid_NonEmpty)
{
    auto id = StringUtils::generateUuid();
    EXPECT_FALSE(id.empty());
    EXPECT_EQ(id.size(), 32u);
}

TEST(StringUtilsTest, GenerateUuid_Unique)
{
    std::set<std::string> ids;
    for (int i = 0; i < 1000; ++i)
    {
        ids.insert(StringUtils::generateUuid());
    }
    EXPECT_EQ(ids.size(), 1000u);
}

TEST(StringUtilsTest, GenerateUuid_HexOnly)
{
    auto id = StringUtils::generateUuid();
    for (char c : id)
    {
        bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
        EXPECT_TRUE(isHex) << "Non-hex character: " << c;
    }
}