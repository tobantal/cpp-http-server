#include <gtest/gtest.h>
#include "RouteMatcher.hpp"

/**
 * @file RouteMatcherTest.cpp
 * @brief Unit-тесты для RouteMatcher
 * @author Anton Tobolkin
 */

// Тест: точное совпадение без wildcard
TEST(RouteMatcherTest, ExactMatch)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/users", "/api/users"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users", "/api/posts"));
}

// Тест: один wildcard
TEST(RouteMatcherTest, SingleWildcard)
{
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/promo"));
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/docs"));
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/r/abc/123")); // два сегмента
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/api/users"));
}

// Тест: wildcard в середине
TEST(RouteMatcherTest, MiddleWildcard)
{
    EXPECT_TRUE(RouteMatcher::matches("/api/*/details", "/api/users/details"));
    EXPECT_TRUE(RouteMatcher::matches("/api/*/details", "/api/posts/details"));
    EXPECT_FALSE(RouteMatcher::matches("/api/*/details", "/api/users/123"));
}

// Тест: несколько wildcards
TEST(RouteMatcherTest, MultipleWildcards)
{
    EXPECT_TRUE(RouteMatcher::matches("/*/users/*", "/api/users/123"));
    EXPECT_TRUE(RouteMatcher::matches("/*/users/*", "/v1/users/456"));
    EXPECT_FALSE(RouteMatcher::matches("/*/users/*", "/api/posts/123"));
}

// Тест: пустой путь
TEST(RouteMatcherTest, EmptyPath)
{
    EXPECT_TRUE(RouteMatcher::matches("", ""));
    EXPECT_FALSE(RouteMatcher::matches("/r/*", ""));
    EXPECT_FALSE(RouteMatcher::matches("", "/r/promo"));
}

// Тест: разное количество сегментов
TEST(RouteMatcherTest, DifferentSegmentCount)
{
    EXPECT_FALSE(RouteMatcher::matches("/r/*", "/r"));
    EXPECT_FALSE(RouteMatcher::matches("/r", "/r/promo"));
    EXPECT_FALSE(RouteMatcher::matches("/api/users/*", "/api/users/123/edit"));
}

// Тест: trailing slash
TEST(RouteMatcherTest, TrailingSlash)
{
    EXPECT_TRUE(RouteMatcher::matches("/r/*", "/r/promo/"));
    EXPECT_FALSE(RouteMatcher::matches("/r/", "/r"));
}