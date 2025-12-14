#include <gtest/gtest.h>
#include "SimpleRequest.hpp"

/**
 * @file SimpleRequestTest.cpp
 * @brief Unit-тесты для SimpleRequest
 */

// Проверка базовой инициализации и геттеров
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

    auto params = req.getParams();
    EXPECT_TRUE(params.empty());
}

// Проверка создания без заголовков
TEST(SimpleRequestTest, NoHeaders)
{
    SimpleRequest req("GET", "/status", "", "192.168.0.1", 80);

    EXPECT_EQ(req.getMethod(), "GET");
    EXPECT_EQ(req.getPath(), "/status");
    EXPECT_EQ(req.getBody(), "");
    EXPECT_EQ(req.getIp(), "192.168.0.1");
    EXPECT_EQ(req.getPort(), 80);

    auto reqHeaders = req.getHeaders();
    EXPECT_TRUE(reqHeaders.empty());

    auto params = req.getParams();
    EXPECT_TRUE(params.empty());
}

// Проверка, что изменение внешней карты headers не влияет на SimpleRequest
TEST(SimpleRequestTest, HeadersAreCopied)
{
    std::map<std::string, std::string> headers = {{"Accept", "text/plain"}};
    SimpleRequest req("GET", "/info", "", "10.0.0.1", 1234, headers);

    headers["Accept"] = "application/json"; // меняем внешнюю карту

    auto reqHeaders = req.getHeaders();
    EXPECT_EQ(reqHeaders["Accept"], "text/plain"); // должно остаться прежним
}
