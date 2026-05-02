#include <gtest/gtest.h>
#include "adapters/primary/RouteMatcher.hpp"

TEST(RouteMatcherTest, ExactMatch)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/users", "/api/users"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users", "/api/posts"));
}

TEST(RouteMatcherTest, SingleWildcard)
{
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/promo"));
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/docs"));
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/r/abc/123"));
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/api/users"));
}

TEST(RouteMatcherTest, MiddleWildcard)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/*/details", "/api/users/details"));
    EXPECT_TRUE(RouteMatcher::matches("/api/*/details", "/api/posts/details"));
    EXPECT_FALSE(RouteMatcher::matches("/api/*/details", "/api/users/123"));
}

TEST(RouteMatcherTest, MultipleWildcards)
{
    EXPECT_TRUE(RouteMatcher::matches("/*/users/*", "/api/users/123"));
    EXPECT_TRUE(RouteMatcher::matches("/*/users/*", "/v1/users/456"));
    EXPECT_FALSE(RouteMatcher::matches("/*/users/*", "/api/posts/123"));
}

TEST(RouteMatcherTest, EmptyPath)
{
    EXPECT_TRUE(RouteMatcher::matches("", ""));
    EXPECT_FALSE(RouteMatcher::matches("/r/*", ""));
    EXPECT_FALSE(RouteMatcher::matches("", "/r/promo"));
}

TEST(RouteMatcherTest, DifferentSegmentCount)
{
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/r"));
    EXPECT_FALSE(RouteMatcher::matches("/r", "/r/promo"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users/*", "/api/users/123/edit"));
}

TEST(RouteMatcherTest, TrailingSlash)
{
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/promo/"));
    EXPECT_TRUE(RouteMatcher::matches("/r/", "/r"));
}

TEST(RouteMatcherTest, NamedParamSingle)
{
    EXPECT_TRUE(RouteMatcher::matches("/users/:id", "/users/42"));
    EXPECT_TRUE(RouteMatcher::matches("/users/:id", "/users/abc"));
    EXPECT_FALSE(RouteMatcher::matches("/users/:id", "/users/42/edit"));
    EXPECT_FALSE(RouteMatcher::matches("/users/:id", "/posts/42"));
}

TEST(RouteMatcherTest, NamedParamMultiple)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/:version/users/:id", "/api/v1/users/42"));
    EXPECT_TRUE(RouteMatcher::matches("/api/:version/users/:id", "/api/v2/users/abc"));
    EXPECT_FALSE(RouteMatcher::matches("/api/:version/users/:id", "/api/v1/posts/42"));
}

TEST(RouteMatcherTest, MixedNamedParamAndWildcard)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/:version/*", "/api/v1/anything"));
    EXPECT_TRUE(RouteMatcher::matches("/:module/:action/*", "/users/edit/123"));
}

TEST(RouteMatcherTest, NamedParamInMiddle)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/:version/details", "/api/v1/details"));
    EXPECT_TRUE(RouteMatcher::matches("/api/:version/details", "/api/v2/details"));
    EXPECT_FALSE(RouteMatcher::matches("/api/:version/details", "/api/v1/other"));
}

TEST(RouteMatcherTest, IsWildcard)
{
    EXPECT_TRUE(RouteMatcher::isWildcard("*"));
    EXPECT_TRUE(RouteMatcher::isWildcard(":id"));
    EXPECT_TRUE(RouteMatcher::isWildcard(":userId"));
    EXPECT_FALSE(RouteMatcher::isWildcard("users"));
    EXPECT_FALSE(RouteMatcher::isWildcard(":"));
    EXPECT_FALSE(RouteMatcher::isWildcard(""));
}

TEST(RouteMatcherTest, IsNamedParam)
{
    EXPECT_TRUE(RouteMatcher::isNamedParam(":id"));
    EXPECT_TRUE(RouteMatcher::isNamedParam(":userId"));
    EXPECT_FALSE(RouteMatcher::isNamedParam("*"));
    EXPECT_FALSE(RouteMatcher::isNamedParam("users"));
    EXPECT_FALSE(RouteMatcher::isNamedParam(":"));
}

TEST(RouteMatcherTest, ParamName)
{
    EXPECT_EQ(RouteMatcher::paramName(":id"), "id");
    EXPECT_EQ(RouteMatcher::paramName(":userId"), "userId");
    EXPECT_EQ(RouteMatcher::paramName(":version"), "version");
    EXPECT_EQ(RouteMatcher::paramName("users"), "");
    EXPECT_EQ(RouteMatcher::paramName("*"), "");
}
