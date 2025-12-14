#include <gtest/gtest.h>
#include <boost/beast/http.hpp>
#include "BeastRequestAdapter.hpp"

/**
 * @file BeastRequestAdapterTest.cpp
 * @brief Unit-тесты для BeastRequestAdapter
 */

// Тест: корректный путь без query-параметров
TEST(BeastRequestAdapterTest, GetPathWithoutQuery)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/api/users", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPath(), "/api/users");
}

// Тест: путь с query-параметрами → должен обрезать всё после '?'
TEST(BeastRequestAdapterTest, GetPathWithQuery)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/api/users?id=10&sort=asc", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPath(), "/api/users");
}

// Тест: метод запроса
TEST(BeastRequestAdapterTest, GetMethod)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::post, "/submit", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getMethod(), "POST");
}

// Тест: тело запроса
TEST(BeastRequestAdapterTest, GetBody)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::post, "/submit", 11};
    req.body() = "hello-world";
    req.prepare_payload();

    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getBody(), "hello-world");
}

// Тест: парсинг query-параметров
TEST(BeastRequestAdapterTest, GetParams)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/search?q=test&page=2", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto params = adapter.getParams();
    ASSERT_EQ(params.size(), 2u);

    EXPECT_EQ(params["q"], "test");
    EXPECT_EQ(params["page"], "2");
}

// Тест: когда query отсутствует → пустая map
TEST(BeastRequestAdapterTest, GetParamsEmpty)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/hello", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    auto params = adapter.getParams();
    EXPECT_TRUE(params.empty());
}

// Тест: получение заголовков
TEST(BeastRequestAdapterTest, GetHeaders)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/test", 11};
    req.set(http::field::host, "example.com");
    req.set(http::field::user_agent, "BeastTestClient");

    BeastRequestAdapter adapter(req, "127.0.0.1");
    auto headers = adapter.getHeaders();

    ASSERT_EQ(headers.size(), 2u);
    EXPECT_EQ(headers["Host"], "example.com");
    EXPECT_EQ(headers["User-Agent"], "BeastTestClient");
}

// Тест: IP клиента
TEST(BeastRequestAdapterTest, GetIp)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "10.10.10.1");

    EXPECT_EQ(adapter.getIp(), "10.10.10.1");
}

// Тест: порт (жёстко зашит в 80)
TEST(BeastRequestAdapterTest, GetPort)
{
    namespace http = boost::beast::http;

    http::request<http::string_body> req{http::verb::get, "/", 11};
    BeastRequestAdapter adapter(req, "127.0.0.1");

    EXPECT_EQ(adapter.getPort(), 80);
}
