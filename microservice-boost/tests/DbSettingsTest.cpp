#include <gtest/gtest.h>
#include <memory>

#include "settings/DbSettings.hpp"
#include "Environment.hpp"

/**
 * @file DbSettingsTest.cpp
 * @brief Unit-тесты для DbSettings с использованием реального Environment
 */

// Успешная загрузка всех параметров
TEST(DbSettingsTest, LoadSuccess)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.port", 5432);
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.user", std::string("admin"));
    env->setProperty("db.password", std::string("secret"));

    DbSettings settings(env);

    EXPECT_EQ(settings.getHost(), "localhost");
    EXPECT_EQ(settings.getPort(), 5432);
    EXPECT_EQ(settings.getName(), "appdb");
    EXPECT_EQ(settings.getUser(), "admin");
    EXPECT_EQ(settings.getPassword(), "secret");
}

// ===== Проверка отсутствующих параметров =====

TEST(DbSettingsTest, MissingHost)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.port", 5432);
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.user", std::string("admin"));
    env->setProperty("db.password", std::string("secret"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}

TEST(DbSettingsTest, MissingPort)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.user", std::string("admin"));
    env->setProperty("db.password", std::string("secret"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}

TEST(DbSettingsTest, MissingName)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.port", 5432);
    env->setProperty("db.user", std::string("admin"));
    env->setProperty("db.password", std::string("secret"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}

TEST(DbSettingsTest, MissingUser)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.port", 5432);
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.password", std::string("secret"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}

TEST(DbSettingsTest, MissingPassword)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.port", 5432);
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.user", std::string("admin"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}

// Ошибочный тип: port = "abc"
TEST(DbSettingsTest, InvalidPortType)
{
    auto env = std::make_shared<Environment>();
    env->setProperty("db.host", std::string("localhost"));
    env->setProperty("db.port", std::string("invalid"));
    env->setProperty("db.name", std::string("appdb"));
    env->setProperty("db.user", std::string("admin"));
    env->setProperty("db.password", std::string("secret"));

    EXPECT_THROW({
        DbSettings settings(env);
    }, std::runtime_error);
}
