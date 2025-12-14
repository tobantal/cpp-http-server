#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "BeastResponseAdapter.hpp"

/**
 * @file BeastResponseAdapterTest.cpp
 * @brief Unit-тесты для BeastResponseAdapter
 */

// Тест: установка статус-кода
TEST(BeastResponseAdapterTest, SetStatus)
{
    namespace http = boost::beast::http;

    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(404);

    EXPECT_EQ(res.result_int(), 404);
}

// Тест: установка тела ответа
TEST(BeastResponseAdapterTest, SetBody)
{
    namespace http = boost::beast::http;

    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setBody("hello");

    EXPECT_EQ(res.body(), "hello");
}

// Тест: установка заголовка
TEST(BeastResponseAdapterTest, SetHeader)
{
    namespace http = boost::beast::http;

    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setHeader("Content-Type", "application/json");
    adapter.setHeader("X-Test", "42");

    EXPECT_EQ(res["Content-Type"], "application/json");
    EXPECT_EQ(res["X-Test"], "42");
}

// Комбинированный тест: статус + заголовки + тело
TEST(BeastResponseAdapterTest, Combined)
{
    namespace http = boost::beast::http;

    http::response<http::string_body> res;
    BeastResponseAdapter adapter(res);

    adapter.setStatus(200);
    adapter.setHeader("Server", "MyServer");
    adapter.setBody("OK");

    EXPECT_EQ(res.result_int(), 200);
    EXPECT_EQ(res["Server"], "MyServer");
    EXPECT_EQ(res.body(), "OK");
}
