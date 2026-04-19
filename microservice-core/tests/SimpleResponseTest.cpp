#include <gtest/gtest.h>
#include "SimpleResponse.hpp"
#include "HttpStatus.hpp"

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
    IResponse* res = new SimpleResponse();
    
    res->setStatus(201);
    res->setBody("Created");
    res->setHeader("Location", "/api/resource/123");
    res->setResult(500, "application/json", R"({"error": "Internal error"})");

    EXPECT_EQ(res->getStatus(), 500);
    EXPECT_EQ(res->getBody(), R"({"error": "Internal error"})");
    
    auto headers = res->getHeaders();
    EXPECT_EQ(headers.size(), 2u);
    
    delete res;
}

// =============================================================================
// HttpStatus ENUM TESTS
// =============================================================================

TEST(HttpStatusTest, EnumValuesCanBeConvertedToInt)
{
    EXPECT_EQ(toInt(HttpStatus::Ok), 200);
    EXPECT_EQ(toInt(HttpStatus::Created), 201);
    EXPECT_EQ(toInt(HttpStatus::NoContent), 204);
    EXPECT_EQ(toInt(HttpStatus::BadRequest), 400);
    EXPECT_EQ(toInt(HttpStatus::Unauthorized), 401);
    EXPECT_EQ(toInt(HttpStatus::Forbidden), 403);
    EXPECT_EQ(toInt(HttpStatus::NotFound), 404);
    EXPECT_EQ(toInt(HttpStatus::MethodNotAllowed), 405);
    EXPECT_EQ(toInt(HttpStatus::Conflict), 409);
    EXPECT_EQ(toInt(HttpStatus::PayloadTooLarge), 413);
    EXPECT_EQ(toInt(HttpStatus::UnprocessableEntity), 422);
    EXPECT_EQ(toInt(HttpStatus::InternalServerError), 500);
    EXPECT_EQ(toInt(HttpStatus::ServiceUnavailable), 503);
    EXPECT_EQ(toInt(HttpStatus::GatewayTimeout), 504);
}

TEST(HttpStatusTest, GetReasonPhrase_IntOverload)
{
    EXPECT_EQ(getReasonPhrase(200), "OK");
    EXPECT_EQ(getReasonPhrase(201), "Created");
    EXPECT_EQ(getReasonPhrase(204), "No Content");
    EXPECT_EQ(getReasonPhrase(400), "Bad Request");
    EXPECT_EQ(getReasonPhrase(401), "Unauthorized");
    EXPECT_EQ(getReasonPhrase(403), "Forbidden");
    EXPECT_EQ(getReasonPhrase(404), "Not Found");
    EXPECT_EQ(getReasonPhrase(405), "Method Not Allowed");
    EXPECT_EQ(getReasonPhrase(409), "Conflict");
    EXPECT_EQ(getReasonPhrase(413), "Payload Too Large");
    EXPECT_EQ(getReasonPhrase(422), "Unprocessable Entity");
    EXPECT_EQ(getReasonPhrase(500), "Internal Server Error");
    EXPECT_EQ(getReasonPhrase(503), "Service Unavailable");
    EXPECT_EQ(getReasonPhrase(504), "Gateway Timeout");
}

TEST(HttpStatusTest, GetReasonPhrase_EnumOverload)
{
    EXPECT_EQ(getReasonPhrase(HttpStatus::Ok), "OK");
    EXPECT_EQ(getReasonPhrase(HttpStatus::Created), "Created");
    EXPECT_EQ(getReasonPhrase(HttpStatus::NotFound), "Not Found");
    EXPECT_EQ(getReasonPhrase(HttpStatus::InternalServerError), "Internal Server Error");
}

TEST(HttpStatusTest, GetReasonPhrase_UnknownCode)
{
    EXPECT_EQ(getReasonPhrase(999), "Unknown");
}

// =============================================================================
// HttpStatus IN SimpleResponse TESTS
// =============================================================================

TEST(SimpleResponseTest, SetStatusWithEnum)
{
    SimpleResponse res;
    res.setStatus(HttpStatus::NotFound);
    EXPECT_EQ(res.getStatus(), 404);
}

TEST(SimpleResponseTest, SetResultWithEnum)
{
    SimpleResponse res;
    res.setResult(HttpStatus::Created, "application/json", R"({"id": "new"})");

    EXPECT_EQ(res.getStatus(), 201);
    EXPECT_EQ(*res.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(res.getBody(), R"({"id": "new"})");
}

TEST(SimpleResponseTest, SetResultWithEnumConflict)
{
    SimpleResponse res;
    res.setResult(HttpStatus::Conflict, "application/json", R"({"error": "duplicate"})");

    EXPECT_EQ(res.getStatus(), 409);
    EXPECT_EQ(res.getBody(), R"({"error": "duplicate"})");
}

// =============================================================================
// setCookie TESTS
// =============================================================================

TEST(SimpleResponseTest, SetCookie_Basic)
{
    SimpleResponse res;
    res.setCookie("session", "abc123");

    auto cookie = res.getHeader("Set-Cookie");
    ASSERT_TRUE(cookie.has_value());
    EXPECT_EQ(*cookie, "session=abc123; Path=/; HttpOnly");
}

TEST(SimpleResponseTest, SetCookie_WithMaxAge)
{
    SimpleResponse res;
    res.setCookie("token", "xyz", "/", true, false, 3600);

    auto cookie = res.getHeader("Set-Cookie");
    ASSERT_TRUE(cookie.has_value());
    EXPECT_EQ(*cookie, "token=xyz; Path=/; Max-Age=3600; HttpOnly");
}

TEST(SimpleResponseTest, SetCookie_SecureFlag)
{
    SimpleResponse res;
    res.setCookie("sid", "val", "/", true, true, 0);

    auto cookie = res.getHeader("Set-Cookie");
    ASSERT_TRUE(cookie.has_value());
    EXPECT_EQ(*cookie, "sid=val; Path=/; Max-Age=0; HttpOnly; Secure");
}

TEST(SimpleResponseTest, SetCookie_NoHttpOnlyNoSecure)
{
    SimpleResponse res;
    res.setCookie("pref", "dark", "/", false, false, -1);

    auto cookie = res.getHeader("Set-Cookie");
    ASSERT_TRUE(cookie.has_value());
    EXPECT_EQ(*cookie, "pref=dark; Path=/");
}
