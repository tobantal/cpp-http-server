#include <gtest/gtest.h>
#include <memory>

#include "settings/ServerSettings.hpp"
#include "Environment.hpp"

/**
 * @file ServerSettingsTest.cpp
 * @brief Unit-тесты для ServerSettings с использованием реального Environment
 */

// Успешная загрузка настроек
TEST(ServerSettingsTest, LoadSuccess)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("127.0.0.1"));
    env->setProperty("server.port", 8080);

    ServerSettings settings(env);

    EXPECT_EQ(settings.getHost(), "127.0.0.1");
    EXPECT_EQ(settings.getPort(), 8080);
}

// Ошибка: отсутствует server.host
TEST(ServerSettingsTest, MissingHost)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("server.port", 8080);

    EXPECT_THROW({
        ServerSettings settings(env);
    }, std::runtime_error);
}

// Ошибка: отсутствует server.port
TEST(ServerSettingsTest, MissingPort)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("localhost"));

    EXPECT_THROW({
        ServerSettings settings(env);
    }, std::runtime_error);
}

// Ошибка: неправильный тип для server.port (например string)
TEST(ServerSettingsTest, InvalidPortType)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", std::string("localhost"));
    env->setProperty("server.port", std::string("not_number"));

    EXPECT_THROW({
        ServerSettings settings(env);
    }, std::runtime_error);
}

// Ошибка: неправильный тип для server.host (например int)
TEST(ServerSettingsTest, InvalidHostType)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("server.host", 12345);
    env->setProperty("server.port", 8080);
    EXPECT_THROW({
        ServerSettings settings(env);
    }, std::runtime_error);
}