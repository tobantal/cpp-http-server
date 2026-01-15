#include <gtest/gtest.h>
#include "SimpleResponse.hpp"

/**
 * @file SimpleResponseTest.cpp
 * @brief Unit-тесты для SimpleResponse v2
 */

// =============================================================================
// BASIC TESTS
// =============================================================================

TEST(SimpleResponseTest, DefaultConstructor)
{
    SimpleResponse res;

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), "");
    EXPECT_TRUE(res.getHeaders().empty());
}

TEST(SimpleResponseTest, ConstructorWithParams)
{
    SimpleResponse res(404, "Not Found");

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), "Not Found");
}

// =============================================================================
// SETTERS TESTS
// =============================================================================

TEST(SimpleResponseTest, SetStatusAndBody)
{
    SimpleResponse res;

    res.setStatus(404);
    res.setBody("Not Found");

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), "Not Found");
}

TEST(SimpleResponseTest, SetHeader)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "application/json");

    auto headers = res.getHeaders();
    ASSERT_EQ(headers.size(), 1u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
}

TEST(SimpleResponseTest, OverwriteHeader)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Type", "application/json");

    auto headers = res.getHeaders();
    ASSERT_EQ(headers.size(), 1u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
}

TEST(SimpleResponseTest, MultipleHeaders)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "application/json");
    res.setHeader("Cache-Control", "no-cache");

    auto headers = res.getHeaders();
    ASSERT_EQ(headers.size(), 2u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
    EXPECT_EQ(headers["Cache-Control"], "no-cache");
}

// =============================================================================
// GETTERS TESTS
// =============================================================================

TEST(SimpleResponseTest, GetHeaderCaseInsensitive)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "application/json");

    auto ct1 = res.getHeader("Content-Type");
    auto ct2 = res.getHeader("content-type");
    auto ct3 = res.getHeader("CONTENT-TYPE");

    ASSERT_TRUE(ct1.has_value());
    ASSERT_TRUE(ct2.has_value());
    ASSERT_TRUE(ct3.has_value());
    EXPECT_EQ(*ct1, "application/json");
    EXPECT_EQ(*ct2, "application/json");
    EXPECT_EQ(*ct3, "application/json");
}

TEST(SimpleResponseTest, GetHeaderNotFound)
{
    SimpleResponse res;

    auto header = res.getHeader("X-Missing");
    EXPECT_FALSE(header.has_value());
}

// =============================================================================
// CONVENIENCE METHODS TESTS
// =============================================================================

TEST(SimpleResponseTest, SetResult)
{
    SimpleResponse res;

    res.setResult(200, "application/json", R"({"status": "ok"})");

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(*res.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(res.getBody(), R"({"status": "ok"})");
}

TEST(SimpleResponseTest, SetResultError)
{
    SimpleResponse res;

    res.setResult(404, "application/json", R"({"error": "Not found"})");

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(*res.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(res.getBody(), R"({"error": "Not found"})");
}

TEST(SimpleResponseTest, SetResultPlainText)
{
    SimpleResponse res;

    res.setResult(200, "text/plain", "Hello, World!");

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(*res.getHeader("Content-Type"), "text/plain");
    EXPECT_EQ(res.getBody(), "Hello, World!");
}

TEST(SimpleResponseTest, SetResultOverwrite)
{
    SimpleResponse res;

    res.setResult(200, "application/json", "first");
    res.setResult(400, "text/plain", "second");

    EXPECT_EQ(res.getStatus(), 400);
    EXPECT_EQ(*res.getHeader("Content-Type"), "text/plain");
    EXPECT_EQ(res.getBody(), "second");
}

// =============================================================================
// INTERFACE COMPLIANCE TESTS
// =============================================================================

TEST(SimpleResponseTest, InterfaceCompliance)
{
    // Test that SimpleResponse implements IResponse correctly
    IResponse* res = new SimpleResponse();
    
    res->setStatus(201);
    res->setBody("Created");
    res->setHeader("Location", "/api/resource/123");
    res->setResult(500, "application/json", R"({"error": "Internal error"})");

    EXPECT_EQ(res->getStatus(), 500);
    EXPECT_EQ(res->getBody(), R"({"error": "Internal error"})");
    
    auto headers = res->getHeaders();
    EXPECT_EQ(headers.size(), 2u); // Content-Type and Location
    
    delete res;
}
