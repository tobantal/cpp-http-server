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
