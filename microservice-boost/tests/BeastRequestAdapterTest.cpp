#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "adapters/primary/BeastRequestAdapter.hpp"

/**
 * @file BeastRequestAdapterTest.cpp
 * @brief Unit-тесты для BeastRequestAdapter v2
 */

namespace http = boost::beast::http;

// =============================================================================
// PATH TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetPathWithoutQuery)
{
    http::request<http::string_body> req{http::verb::get, "/api/users", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPath(), "/api/users");
}

TEST(BeastRequestAdapterTest, GetPathWithQuery)
{
    http::request<http::string_body> req{http::verb::get, "/api/users?id=10&sort=asc", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPath(), "/api/users");
}

TEST(BeastRequestAdapterTest, GetPathSegments)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/orders/ord-123", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto segments = adapter.getPathSegments();
    ASSERT_EQ(segments.size(), 4u);
    EXPECT_EQ(segments[0], "api");
    EXPECT_EQ(segments[1], "v1");
    EXPECT_EQ(segments[2], "orders");
    EXPECT_EQ(segments[3], "ord-123");
}

TEST(BeastRequestAdapterTest, GetPathSegmentsEmpty)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto segments = adapter.getPathSegments();
    EXPECT_TRUE(segments.empty());
}

// =============================================================================
// PATH PARAMETERS TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetPathPattern)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/orders/ord-123", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPathPattern(), "");

    adapter.setPathPattern("/api/v1/orders/*");
    EXPECT_EQ(adapter.getPathPattern(), "/api/v1/orders/*");
}

TEST(BeastRequestAdapterTest, GetPathParamSingleWildcard)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/orders/ord-123", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    adapter.setPathPattern("/api/v1/orders/*");

    auto param0 = adapter.getPathParam(0);
    ASSERT_TRUE(param0.has_value());
    EXPECT_EQ(*param0, "ord-123");

    auto param1 = adapter.getPathParam(1);
    EXPECT_FALSE(param1.has_value());
}

TEST(BeastRequestAdapterTest, GetPathParamMultipleWildcards)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/orders/ord-123/items/item-456", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    adapter.setPathPattern("/api/v1/orders/*/items/*");

    auto param0 = adapter.getPathParam(0);
    ASSERT_TRUE(param0.has_value());
    EXPECT_EQ(*param0, "ord-123");

    auto param1 = adapter.getPathParam(1);
    ASSERT_TRUE(param1.has_value());
    EXPECT_EQ(*param1, "item-456");

    auto param2 = adapter.getPathParam(2);
    EXPECT_FALSE(param2.has_value());
}

TEST(BeastRequestAdapterTest, GetPathParamNoPattern)
{
    http::request<http::string_body> req{http::verb::get, "/api/orders/123", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto param = adapter.getPathParam(0);
    EXPECT_FALSE(param.has_value());
}

// =============================================================================
// QUERY PARAMETERS TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetQueryParams)
{
    http::request<http::string_body> req{http::verb::get, "/search?q=test&page=2", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto params = adapter.getQueryParams();
    ASSERT_EQ(params.size(), 2u);
    EXPECT_EQ(params["q"], "test");
    EXPECT_EQ(params["page"], "2");
}

TEST(BeastRequestAdapterTest, GetQueryParam)
{
    http::request<http::string_body> req{http::verb::get, "/search?q=test&page=2", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto q = adapter.getQueryParam("q");
    ASSERT_TRUE(q.has_value());
    EXPECT_EQ(*q, "test");

    auto missing = adapter.getQueryParam("missing");
    EXPECT_FALSE(missing.has_value());
}

TEST(BeastRequestAdapterTest, SetQueryParam)
{
    http::request<http::string_body> req{http::verb::get, "/search?q=test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    adapter.setQueryParam("limit", "100");

    auto limit = adapter.getQueryParam("limit");
    ASSERT_TRUE(limit.has_value());
    EXPECT_EQ(*limit, "100");
}

TEST(BeastRequestAdapterTest, GetParamsEmpty)
{
    http::request<http::string_body> req{http::verb::get, "/hello", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto params = adapter.getQueryParams();
    EXPECT_TRUE(params.empty());
}

// Deprecated alias test
TEST(BeastRequestAdapterTest, GetParamsAlias)
{
    http::request<http::string_body> req{http::verb::get, "/search?q=test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto params = adapter.getParams();
    EXPECT_EQ(params["q"], "test");
}

// =============================================================================
// HEADERS TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetHeaders)
{
    http::request<http::string_body> req{http::verb::get, "/test", 11};
    req.set(http::field::host, "example.com");
    req.set(http::field::user_agent, "BeastTestClient");

    BeastRequestAdapter adapter(req, "127.0.0.1");
    auto headers = adapter.getHeaders();

    ASSERT_EQ(headers.size(), 2u);
    EXPECT_EQ(headers["Host"], "example.com");
    EXPECT_EQ(headers["User-Agent"], "BeastTestClient");
}

TEST(BeastRequestAdapterTest, GetHeaderCaseInsensitive)
{
    http::request<http::string_body> req{http::verb::get, "/test", 11};
    req.set(http::field::content_type, "application/json");

    BeastRequestAdapter adapter(req, "127.0.0.1");

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

TEST(BeastRequestAdapterTest, GetHeaderNotFound)
{
    http::request<http::string_body> req{http::verb::get, "/test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto header = adapter.getHeader("X-Missing");
    EXPECT_FALSE(header.has_value());
}

TEST(BeastRequestAdapterTest, SetHeader)
{
    http::request<http::string_body> req{http::verb::get, "/test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    adapter.setHeader("X-Custom", "value123");

    auto header = adapter.getHeader("X-Custom");
    ASSERT_TRUE(header.has_value());
    EXPECT_EQ(*header, "value123");
}

TEST(BeastRequestAdapterTest, SetHeaders)
{
    http::request<http::string_body> req{http::verb::get, "/test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    std::map<std::string, std::string> headers = {
        {"X-Header1", "value1"},
        {"X-Header2", "value2"}
    };
    adapter.setHeaders(headers);

    EXPECT_EQ(*adapter.getHeader("X-Header1"), "value1");
    EXPECT_EQ(*adapter.getHeader("X-Header2"), "value2");
}

// =============================================================================
// BODY TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetBody)
{
    http::request<http::string_body> req{http::verb::post, "/submit", 11};
    req.body() = "hello-world";
    req.prepare_payload();

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getBody(), "hello-world");
}

TEST(BeastRequestAdapterTest, SetBody)
{
    http::request<http::string_body> req{http::verb::post, "/submit", 11};
    req.body() = "original";

    BeastRequestAdapter adapter(req, "127.0.0.1");
    adapter.setBody("modified");

    EXPECT_EQ(adapter.getBody(), "modified");
}

// =============================================================================
// METHOD TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetMethod)
{
    http::request<http::string_body> req{http::verb::post, "/submit", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getMethod(), "POST");
}

// =============================================================================
// CONNECTION INFO TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetIp)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "10.10.10.1");

    EXPECT_EQ(adapter.getIp(), "10.10.10.1");
}

TEST(BeastRequestAdapterTest, GetPortDefault)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPort(), 80);
}

TEST(BeastRequestAdapterTest, GetPortExplicit)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1", 8080);

    EXPECT_EQ(adapter.getPort(), 8080);
}

TEST(BeastRequestAdapterTest, GetPortCustom)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "10.0.0.1", 443);

    EXPECT_EQ(adapter.getPort(), 443);
}

// =============================================================================
// CONVENIENCE METHODS TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetBearerToken)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::authorization, "Bearer eyJhbGciOiJIUzI1NiJ9");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto token = adapter.getBearerToken();
    ASSERT_TRUE(token.has_value());
    EXPECT_EQ(*token, "eyJhbGciOiJIUzI1NiJ9");
}

TEST(BeastRequestAdapterTest, GetBearerTokenNotBearer)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::authorization, "Basic abc123");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto token = adapter.getBearerToken();
    EXPECT_FALSE(token.has_value());
}

TEST(BeastRequestAdapterTest, GetBearerTokenMissing)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto token = adapter.getBearerToken();
    EXPECT_FALSE(token.has_value());
}

TEST(BeastRequestAdapterTest, IsJson)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::content_type, "application/json");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_TRUE(adapter.isJson());
}

TEST(BeastRequestAdapterTest, IsJsonWithCharset)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::content_type, "application/json; charset=utf-8");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_TRUE(adapter.isJson());
}

TEST(BeastRequestAdapterTest, IsJsonFalse)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::content_type, "text/plain");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_FALSE(adapter.isJson());
}

TEST(BeastRequestAdapterTest, GetContentType)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    req.set(http::field::content_type, "text/html");

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getContentType(), "text/html");
}

TEST(BeastRequestAdapterTest, GetContentTypeMissing)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getContentType(), "");
}

// =============================================================================
// ATTRIBUTES TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, SetAndGetAttribute)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    adapter.setAttribute("user_id", "user-123");

    auto attr = adapter.getAttribute("user_id");
    ASSERT_TRUE(attr.has_value());
    EXPECT_EQ(*attr, "user-123");
}

TEST(BeastRequestAdapterTest, GetAttributeNotFound)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto attr = adapter.getAttribute("missing");
    EXPECT_FALSE(attr.has_value());
}

TEST(BeastRequestAdapterTest, MultipleAttributes)
{
    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    adapter.setAttribute("user_id", "user-123");
    adapter.setAttribute("account_id", "acc-456");
    adapter.setAttribute("account_type", "sandbox");

    EXPECT_EQ(*adapter.getAttribute("user_id"), "user-123");
    EXPECT_EQ(*adapter.getAttribute("account_id"), "acc-456");
    EXPECT_EQ(*adapter.getAttribute("account_type"), "sandbox");
}

// =============================================================================
// URL DECODING TESTS
// =============================================================================

TEST(BeastRequestAdapterTest, GetPath_DecodesPercentEncoding)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/my%20order", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    EXPECT_EQ(adapter.getPath(), "/api/v1/my order");
}

TEST(BeastRequestAdapterTest, GetPath_DecodesUnicode)
{
    http::request<http::string_body> req{http::verb::get, "/%C3%BCnicode", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    EXPECT_EQ(adapter.getPath(), "/ünicode");
}

TEST(BeastRequestAdapterTest, GetPath_StripsQueryStringThenDecodes)
{
    http::request<http::string_body> req{http::verb::get, "/api/hello%20world?status=active", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    EXPECT_EQ(adapter.getPath(), "/api/hello world");
}

TEST(BeastRequestAdapterTest, GetPath_PlainPathUnchanged)
{
    http::request<http::string_body> req{http::verb::get, "/api/v1/orders", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    EXPECT_EQ(adapter.getPath(), "/api/v1/orders");
}

TEST(BeastRequestAdapterTest, GetQueryParams_DecodedValues)
{
    http::request<http::string_body> req{http::verb::get, "/api?name=hello%20world&type=test", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    auto params = adapter.getQueryParams();
    EXPECT_EQ(params.at("name"), "hello world");
    EXPECT_EQ(params.at("type"), "test");
}

TEST(BeastRequestAdapterTest, GetQueryParams_PlusDecodedAsSpace)
{
    http::request<http::string_body> req{http::verb::get, "/api?q=hello+world", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");
    auto params = adapter.getQueryParams();
    EXPECT_EQ(params.at("q"), "hello world");
}
