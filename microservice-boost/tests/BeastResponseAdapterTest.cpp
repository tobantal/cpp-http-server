#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "adapters/primary/BeastResponseAdapter.hpp"
#include "domain/HttpStatus.hpp"

/**
 * @file BeastResponseAdapterTest.cpp
 * @brief Unit-тесты для BeastResponseAdapter v2
 */

namespace http = boost::beast::http;

// =============================================================================
// SETTERS TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, SetStatus)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(404);

    EXPECT_EQ(res.result_int(), 404);
}

TEST(BeastResponseAdapterTest, SetBody)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setBody("hello");

    EXPECT_EQ(res.body(), "hello");
}

TEST(BeastResponseAdapterTest, SetHeader)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setHeader("Content-Type", "application/json");
    adapter.setHeader("X-Test", "42");

    EXPECT_EQ(res["Content-Type"], "application/json");
    EXPECT_EQ(res["X-Test"], "42");
}

// =============================================================================
// GETTERS TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, GetStatus)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(201);

    EXPECT_EQ(adapter.getStatus(), 201);
}

TEST(BeastResponseAdapterTest, GetBody)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setBody("test body");

    EXPECT_EQ(adapter.getBody(), "test body");
}

TEST(BeastResponseAdapterTest, GetHeaders)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setHeader("Content-Type", "application/json");
    adapter.setHeader("X-Custom", "value");

    auto headers = adapter.getHeaders();
    EXPECT_EQ(headers.size(), 2u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
    EXPECT_EQ(headers["X-Custom"], "value");
}

TEST(BeastResponseAdapterTest, GetHeaderCaseInsensitive)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setHeader("Content-Type", "application/json");

    auto ct1 = adapter.getHeader("Content-Type");
    auto ct2 = adapter.getHeader("content-type");
    auto ct3 = adapter.getHeader("CONTENT-TYPE");

    ASSERT_TRUE(ct1.has_value());
    ASSERT_TRUE(ct2.has_value());
    ASSERT_TRUE(ct3.has_value());
    EXPECT_EQ(*ct1, "application/json");
    EXPECT_EQ(*ct2, "application/json");
    EXPECT_EQ(*ct3, "application/json");
}

TEST(BeastResponseAdapterTest, GetHeaderNotFound)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    auto header = adapter.getHeader("X-Missing");
    EXPECT_FALSE(header.has_value());
}

// =============================================================================
// CONVENIENCE METHODS TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, SetResult)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setResult(200, "application/json", R"({"status": "ok"})");

    EXPECT_EQ(adapter.getStatus(), 200);
    EXPECT_EQ(*adapter.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(adapter.getBody(), R"({"status": "ok"})");
}

TEST(BeastResponseAdapterTest, SetResultError)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setResult(404, "application/json", R"({"error": "Not found"})");

    EXPECT_EQ(adapter.getStatus(), 404);
    EXPECT_EQ(*adapter.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(adapter.getBody(), R"({"error": "Not found"})");
}

TEST(BeastResponseAdapterTest, SetResultPlainText)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setResult(200, "text/plain", "Hello, World!");

    EXPECT_EQ(adapter.getStatus(), 200);
    EXPECT_EQ(*adapter.getHeader("Content-Type"), "text/plain");
    EXPECT_EQ(adapter.getBody(), "Hello, World!");
}

// =============================================================================
// COMBINED TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, Combined)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(200);
    adapter.setHeader("Server", "MyServer");
    adapter.setBody("OK");

    EXPECT_EQ(res.result_int(), 200);
    EXPECT_EQ(res["Server"], "MyServer");
    EXPECT_EQ(res.body(), "OK");
}

TEST(BeastResponseAdapterTest, OverwriteValues)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setResult(200, "application/json", "first");
    adapter.setResult(400, "text/plain", "second");

    EXPECT_EQ(adapter.getStatus(), 400);
    EXPECT_EQ(*adapter.getHeader("Content-Type"), "text/plain");
    EXPECT_EQ(adapter.getBody(), "second");
}

// =============================================================================
// HttpStatus ENUM TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, SetStatusWithEnum)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(HttpStatus::NotFound);

    EXPECT_EQ(adapter.getStatus(), 404);
}

TEST(BeastResponseAdapterTest, SetResultWithEnum)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setResult(HttpStatus::Created, "application/json", R"({"id": "new"})");

    EXPECT_EQ(adapter.getStatus(), 201);
    EXPECT_EQ(*adapter.getHeader("Content-Type"), "application/json");
}

// =============================================================================
// setCookie TESTS
// =============================================================================

TEST(BeastResponseAdapterTest, SetCookie_Basic)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setCookie("session", "abc123");

    auto cookie = res[http::field::set_cookie];
    EXPECT_EQ(std::string(cookie), "session=abc123; Path=/; HttpOnly");
}

TEST(BeastResponseAdapterTest, SetCookie_WithMaxAge)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setCookie("token", "xyz", "/", true, false, 3600);

    auto cookie = res[http::field::set_cookie];
    EXPECT_EQ(std::string(cookie), "token=xyz; Path=/; Max-Age=3600; HttpOnly");
}

TEST(BeastResponseAdapterTest, SetCookie_SecureFlag)
{
    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setCookie("sid", "val", "/", true, true, 0);

    auto cookie = res[http::field::set_cookie];
    EXPECT_EQ(std::string(cookie), "sid=val; Path=/; Max-Age=0; HttpOnly; Secure");
}
