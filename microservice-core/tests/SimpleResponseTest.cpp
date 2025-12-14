#include <gtest/gtest.h>
#include "SimpleResponse.hpp"

/**
 * @file SimpleResponseTest.cpp
 * @brief Unit-тесты для SimpleResponse
 */

// Проверка дефолтной инициализации
TEST(SimpleResponseTest, DefaultConstructor)
{
    SimpleResponse res;

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), "");
    EXPECT_TRUE(res.getHeaders().empty());
}

// Проверка установки статуса и тела
TEST(SimpleResponseTest, SetStatusAndBody)
{
    SimpleResponse res;

    res.setStatus(404);
    res.setBody("Not Found");

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), "Not Found");
}

// Проверка установки одного заголовка
TEST(SimpleResponseTest, SetHeader)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "application/json");

    auto headers = res.getHeaders();
    ASSERT_EQ(headers.size(), 1u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
}

// Проверка перезаписи заголовка
TEST(SimpleResponseTest, OverwriteHeader)
{
    SimpleResponse res;
    res.setHeader("Content-Type", "text/plain");
    res.setHeader("Content-Type", "application/json"); // перезаписываем

    auto headers = res.getHeaders();
    ASSERT_EQ(headers.size(), 1u);
    EXPECT_EQ(headers["Content-Type"], "application/json");
}

// Проверка установки нескольких заголовков
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
