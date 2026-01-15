#include <gtest/gtest.h>
#include "SimpleRequest.hpp"

/**
 * @file SimpleRequestTest.cpp
 * @brief Unit-тесты для SimpleRequest v2
 */

// =============================================================================
// BASIC TESTS
// =============================================================================

TEST(SimpleRequestTest, BasicProperties)
{
    std::map<std::string, std::string> headers = {{"Content-Type", "application/json"}};
    SimpleRequest req("POST", "/api/data", "{\"key\":42}", "127.0.0.1", 8080, headers);

    EXPECT_EQ(req.getMethod(), "POST");
    EXPECT_EQ(req.getPath(), "/api/data");
    EXPECT_EQ(req.getBody(), "{\"key\":42}");
    EXPECT_EQ(req.getIp(), "127.0.0.1");
    EXPECT_EQ(req.getPort(), 8080);

    auto reqHeaders = req.getHeaders();
    ASSERT_EQ(reqHeaders.size(), 1u);
    EXPECT_EQ(reqHeaders["Content-Type"], "application/json");
}

TEST(SimpleRequestTest, DefaultConstructor)
{
    SimpleRequest req;

    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/");
    EXPECT_EQ(req.getBody(), "");
    EXPECT_EQ(req.getIp(), "127.0.0.1");
    EXPECT_EQ(req.getPort(), 80);
}

TEST(SimpleRequestTest, NoHeaders)
{
    SimpleRequest req("GET", "/status", "", "192.168.0.1", 80);

    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/status");
    EXPECT_TRUE(req.getHeaders().empty());
}

// =============================================================================
// PATH TESTS
// =============================================================================

TEST(SimpleRequestTest, GetPathSegments)
{
    SimpleRequest req("GET", "/api/v1/orders/123", "", "127.0.0.1", 80);

    auto segments = req.getPathSegments();
    ASSERT_EQ(segments.size(), 4u);
    EXPECT_EQ(segments[0], "api");
    EXPECT_EQ(segments[1], "v1");
    EXPECT_EQ(segments[2], "orders");
    EXPECT_EQ(segments[3], "123");
}

TEST(SimpleRequestTest, GetPathSegmentsRoot)
{
    SimpleRequest req("GET", "/", "", "127.0.0.1", 80);

    auto segments = req.getPathSegments();
    EXPECT_TRUE(segments.empty());
}

// =============================================================================
// PATH PARAMETERS TESTS
// =============================================================================

TEST(SimpleRequestTest, PathPattern)
{
    SimpleRequest req("GET", "/api/orders/123", "", "127.0.0.1", 80);

    EXPECT_EQ(req.getPathPattern(), "");

    req.setPathPattern("/api/orders/*");
    EXPECT_EQ(req.getPathPattern(), "/api/orders/*");
}

TEST(SimpleRequestTest, GetPathParam)
{
    SimpleRequest req("GET", "/api/orders/ord-123", "", "127.0.0.1", 80);
    req.setPathPattern("/api/orders/*");

    auto param = req.getPathParam(0);
    ASSERT_TRUE(param.has_value());
    EXPECT_EQ(*param, "ord-123");
}

TEST(SimpleRequestTest, GetPathParamMultiple)
{
    SimpleRequest req("GET", "/api/orders/ord-123/items/item-456", "", "127.0.0.1", 80);
    req.setPathPattern("/api/orders/*/items/*");

    EXPECT_EQ(*req.getPathParam(0), "ord-123");
    EXPECT_EQ(*req.getPathParam(1), "item-456");
    EXPECT_FALSE(req.getPathParam(2).has_value());
}

TEST(SimpleRequestTest, GetPathParamNoPattern)
{
    SimpleRequest req("GET", "/api/orders/123", "", "127.0.0.1", 80);

    EXPECT_FALSE(req.getPathParam(0).has_value());
}

// =============================================================================
// QUERY PARAMETERS TESTS
// =============================================================================

TEST(SimpleRequestTest, QueryParams)
{
    SimpleRequest req;
    
    req.setQueryParam("page", "1");
    req.setQueryParam("limit", "10");

    EXPECT_EQ(*req.getQueryParam("page"), "1");
    EXPECT_EQ(*req.getQueryParam("limit"), "10");
    EXPECT_FALSE(req.getQueryParam("missing").has_value());
}

TEST(SimpleRequestTest, GetQueryParams)
{
    SimpleRequest req;
    
    req.setQueryParam("a", "1");
    req.setQueryParam("b", "2");

    auto params = req.getQueryParams();
    EXPECT_EQ(params.size(), 2u);
    EXPECT_EQ(params["a"], "1");
    EXPECT_EQ(params["b"], "2");
}

// Deprecated alias
TEST(SimpleRequestTest, GetParamsAlias)
{
    SimpleRequest req;
    req.setQueryParam("test", "value");

    auto params = req.getParams();
    EXPECT_EQ(params["test"], "value");
}

// =============================================================================
// HEADERS TESTS
// =============================================================================

TEST(SimpleRequestTest, GetHeaderCaseInsensitive)
{
    SimpleRequest req;
    req.setHeader("Content-Type", "application/json");

    EXPECT_EQ(*req.getHeader("Content-Type"), "application/json");
    EXPECT_EQ(*req.getHeader("content-type"), "application/json");
    EXPECT_EQ(*req.getHeader("CONTENT-TYPE"), "application/json");
}

TEST(SimpleRequestTest, SetHeaders)
{
    SimpleRequest req;
    
    std::map<std::string, std::string> headers = {
        {"X-Header1", "value1"},
        {"X-Header2", "value2"}
    };
    req.setHeaders(headers);

    EXPECT_EQ(*req.getHeader("X-Header1"), "value1");
    EXPECT_EQ(*req.getHeader("X-Header2"), "value2");
}

TEST(SimpleRequestTest, HeadersAreCopied)
{
    std::map<std::string, std::string> headers = {{"Accept", "text/plain"}};
    SimpleRequest req("GET", "/info", "", "10.0.0.1", 1234, headers);

    headers["Accept"] = "application/json";

    auto reqHeaders = req.getHeaders();
    EXPECT_EQ(reqHeaders["Accept"], "text/plain");
}

// =============================================================================
// BODY TESTS
// =============================================================================

TEST(SimpleRequestTest, SetBody)
{
    SimpleRequest req;
    req.setBody("new body content");

    EXPECT_EQ(req.getBody(), "new body content");
}

// =============================================================================
// CONVENIENCE METHODS TESTS
// =============================================================================

TEST(SimpleRequestTest, GetBearerToken)
{
    SimpleRequest req;
    req.setHeader("Authorization", "Bearer abc123");

    auto token = req.getBearerToken();
    ASSERT_TRUE(token.has_value());
    EXPECT_EQ(*token, "abc123");
}

TEST(SimpleRequestTest, GetBearerTokenNotBearer)
{
    SimpleRequest req;
    req.setHeader("Authorization", "Basic xyz");

    EXPECT_FALSE(req.getBearerToken().has_value());
}

TEST(SimpleRequestTest, GetBearerTokenMissing)
{
    SimpleRequest req;
    EXPECT_FALSE(req.getBearerToken().has_value());
}

TEST(SimpleRequestTest, IsJson)
{
    SimpleRequest req;
    req.setHeader("Content-Type", "application/json");
    EXPECT_TRUE(req.isJson());
}

TEST(SimpleRequestTest, IsJsonWithCharset)
{
    SimpleRequest req;
    req.setHeader("Content-Type", "application/json; charset=utf-8");
    EXPECT_TRUE(req.isJson());
}

TEST(SimpleRequestTest, IsJsonFalse)
{
    SimpleRequest req;
    req.setHeader("Content-Type", "text/plain");
    EXPECT_FALSE(req.isJson());
}

TEST(SimpleRequestTest, GetContentType)
{
    SimpleRequest req;
    req.setHeader("Content-Type", "text/html");
    EXPECT_EQ(req.getContentType(), "text/html");
}

TEST(SimpleRequestTest, GetContentTypeMissing)
{
    SimpleRequest req;
    EXPECT_EQ(req.getContentType(), "");
}

// =============================================================================
// ATTRIBUTES TESTS
// =============================================================================

TEST(SimpleRequestTest, Attributes)
{
    SimpleRequest req;
    
    req.setAttribute("user_id", "123");
    req.setAttribute("role", "admin");

    EXPECT_EQ(*req.getAttribute("user_id"), "123");
    EXPECT_EQ(*req.getAttribute("role"), "admin");
    EXPECT_FALSE(req.getAttribute("missing").has_value());
}

// =============================================================================
// SETTERS TESTS
// =============================================================================

TEST(SimpleRequestTest, TestSetters)
{
    SimpleRequest req;
    
    req.setMethod("PUT");
    req.setPath("/updated/path");
    req.setIp("10.0.0.1");
    req.setPort(9000);

    EXPECT_EQ(req.getMethod(), "PUT");
    EXPECT_EQ(req.getPath(), "/updated/path");
    EXPECT_EQ(req.getIp(), "10.0.0.1");
    EXPECT_EQ(req.getPort(), 9000);
}
