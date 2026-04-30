#include <gtest/gtest.h>
#include "adapters/secondary/InMemoryKeyValueRepository.hpp"

class InMemoryKeyValueRepositoryTest : public ::testing::Test
{
protected:
    InMemoryKeyValueRepository repo_;
};

TEST_F(InMemoryKeyValueRepositoryTest, SaveAndFindById)
{
    KeyValueEntity entity{"key1", R"({"name":"Alice"})"};
    repo_.save(entity);

    auto found = repo_.findById("key1");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->id, "key1");
    EXPECT_EQ(found->value, R"({"name":"Alice"})");
}

TEST_F(InMemoryKeyValueRepositoryTest, FindByIdReturnsNulloptForMissing)
{
    auto found = repo_.findById("missing");
    EXPECT_FALSE(found.has_value());
}

TEST_F(InMemoryKeyValueRepositoryTest, SaveUpdatesExisting)
{
    repo_.save({"key1", R"({"name":"Alice"})"});
    repo_.save({"key1", R"({"name":"Bob"})"});

    auto found = repo_.findById("key1");
    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found->value, R"({"name":"Bob"})");
}

TEST_F(InMemoryKeyValueRepositoryTest, FindAllReturnsAll)
{
    repo_.save({"key1", "val1"});
    repo_.save({"key2", "val2"});
    repo_.save({"key3", "val3"});

    auto all = repo_.findAll();
    EXPECT_EQ(all.size(), 3u);
}

TEST_F(InMemoryKeyValueRepositoryTest, FindAllReturnsEmptyWhenNoData)
{
    auto all = repo_.findAll();
    EXPECT_TRUE(all.empty());
}

TEST_F(InMemoryKeyValueRepositoryTest, RemoveByIdDeletesEntity)
{
    repo_.save({"key1", "val1"});
    EXPECT_TRUE(repo_.removeById("key1"));
    EXPECT_FALSE(repo_.findById("key1").has_value());
}

TEST_F(InMemoryKeyValueRepositoryTest, RemoveByIdReturnsFalseForMissing)
{
    EXPECT_FALSE(repo_.removeById("missing"));
}

TEST_F(InMemoryKeyValueRepositoryTest, SaveAllBatchInsert)
{
    std::vector<KeyValueEntity> batch = {
        {"k1", "v1"},
        {"k2", "v2"},
        {"k3", "v3"}
    };
    repo_.saveAll(batch);

    EXPECT_EQ(repo_.size(), 3u);
    EXPECT_EQ(repo_.findById("k2")->value, "v2");
}

TEST_F(InMemoryKeyValueRepositoryTest, SaveAllOverwritesExisting)
{
    repo_.save({"k1", "old"});
    std::vector<KeyValueEntity> batch = {
        {"k1", "new"},
        {"k2", "v2"}
    };
    repo_.saveAll(batch);

    EXPECT_EQ(repo_.size(), 2u);
    EXPECT_EQ(repo_.findById("k1")->value, "new");
}

TEST_F(InMemoryKeyValueRepositoryTest, SaveAllEmptyDoesNothing)
{
    std::vector<KeyValueEntity> empty;
    repo_.saveAll(empty);
    EXPECT_EQ(repo_.size(), 0u);
}

TEST_F(InMemoryKeyValueRepositoryTest, ClearRemovesAll)
{
    repo_.save({"k1", "v1"});
    repo_.save({"k2", "v2"});
    repo_.clear();
    EXPECT_EQ(repo_.size(), 0u);
    EXPECT_TRUE(repo_.findAll().empty());
}

TEST_F(InMemoryKeyValueRepositoryTest, SizeReturnsCount)
{
    EXPECT_EQ(repo_.size(), 0u);
    repo_.save({"k1", "v1"});
    EXPECT_EQ(repo_.size(), 1u);
    repo_.save({"k2", "v2"});
    EXPECT_EQ(repo_.size(), 2u);
    repo_.removeById("k1");
    EXPECT_EQ(repo_.size(), 1u);
}