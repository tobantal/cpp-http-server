#include <gtest/gtest.h>
#include "ThreadSafeMap.hpp"
#include <algorithm>

/**
 * @file ThreadSafeMapTest.cpp
 * @brief Unit-тесты для ThreadSafeMap
 */

// insert + find
TEST(ThreadSafeMapTest, InsertAndFind)
{
    ThreadSafeMap<int, std::string> map;
    map.insert(1, std::make_shared<std::string>("hello"));

    auto v = map.find(1);
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(*v, "hello");
}

// find неизвестного ключа
TEST(ThreadSafeMapTest, FindNonExistent)
{
    ThreadSafeMap<int, std::string> map;
    EXPECT_EQ(map.find(42), nullptr);
}

// contains работает корректно
TEST(ThreadSafeMapTest, ContainsCheck)
{
    ThreadSafeMap<int, std::string> map;
    map.insert(10, std::make_shared<std::string>("value"));

    EXPECT_TRUE(map.contains(10));
    EXPECT_FALSE(map.contains(11));
}

// remove удаляет элемент
TEST(ThreadSafeMapTest, RemoveKey)
{
    ThreadSafeMap<int, std::string> map;
    map.insert(5, std::make_shared<std::string>("abc"));
    EXPECT_TRUE(map.contains(5));

    map.remove(5);

    EXPECT_FALSE(map.contains(5));
    EXPECT_EQ(map.find(5), nullptr);
}

// clear очищает карту
TEST(ThreadSafeMapTest, ClearMap)
{
    ThreadSafeMap<int, std::string> map;
    map.insert(1, std::make_shared<std::string>("a"));
    map.insert(2, std::make_shared<std::string>("b"));

    map.clear();

    EXPECT_FALSE(map.contains(1));
    EXPECT_FALSE(map.contains(2));
    EXPECT_TRUE(map.getAll().empty());
}

// getAll возвращает все значения
TEST(ThreadSafeMapTest, GetAllValues)
{
    ThreadSafeMap<int, std::string> map;
    map.insert(1, std::make_shared<std::string>("a"));
    map.insert(2, std::make_shared<std::string>("b"));
    map.insert(3, std::make_shared<std::string>("c"));

    auto items = map.getAll();
    ASSERT_EQ(items.size(), 3u);

    std::vector<std::string> vals;
    for (auto &p : items) vals.push_back(*p);

    EXPECT_NE(std::find(vals.begin(), vals.end(), "a"), vals.end());
    EXPECT_NE(std::find(vals.begin(), vals.end(), "b"), vals.end());
    EXPECT_NE(std::find(vals.begin(), vals.end(), "c"), vals.end());
}

// перезапись существующего ключа
TEST(ThreadSafeMapTest, OverwriteExistingKey)
{
    ThreadSafeMap<int, std::string> map;

    map.insert(7, std::make_shared<std::string>("old"));
    map.insert(7, std::make_shared<std::string>("new"));

    auto v = map.find(7);
    ASSERT_NE(v, nullptr);
    EXPECT_EQ(*v, "new");
}
