#include <gtest/gtest.h>
#include "util/Uuid7Generator.hpp"
#include "util/UuidGenerator.hpp"
#include <set>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <cctype>

TEST(Uuid7GeneratorTest, Generate_Format_36CharsWithHyphens)
{
    Uuid7Generator gen;
    auto id = gen.generate();

    ASSERT_EQ(id.size(), 36u) << "UUIDv7 must be 36 characters (32 hex + 4 hyphens)";

    EXPECT_EQ(id[8], '-');
    EXPECT_EQ(id[13], '-');
    EXPECT_EQ(id[18], '-');
    EXPECT_EQ(id[23], '-');
}

TEST(Uuid7GeneratorTest, Generate_Version7)
{
    Uuid7Generator gen;
    auto id = gen.generate();

    // UUIDv7: version nibble is at position 14 (after first hyphen group + hyphen)
    // Format: xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx
    // Position 14 should be '7'
    EXPECT_EQ(id[14], '7') << "UUIDv7 must have version nibble '7' at position 14";
}

TEST(Uuid7GeneratorTest, Generate_VariantRFC4122)
{
    Uuid7Generator gen;
    for (int i = 0; i < 100; ++i)
    {
        auto id = gen.generate();

        // variant bits are at position 19 (first nibble of 3rd group after version)
        // Format: xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx
        // position 19 must be '8', '9', 'a', or 'b'
        char v = id[19];
        bool valid = (v == '8' || v == '9' || v == 'a' || v == 'b');
        EXPECT_TRUE(valid) << "Variant nibble at position 19 must be 8/9/a/b, got: " << v;
    }
}

TEST(Uuid7GeneratorTest, Generate_HexCharactersOnly)
{
    Uuid7Generator gen;

    for (int i = 0; i < 100; ++i)
    {
        auto id = gen.generate();
        for (size_t j = 0; j < id.size(); ++j)
        {
            char c = id[j];
            if (j == 8 || j == 13 || j == 18 || j == 23)
            {
                EXPECT_EQ(c, '-') << "Hyphen expected at position " << j;
            }
            else
            {
                bool isHex = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
                EXPECT_TRUE(isHex) << "Non-hex character '" << c << "' at position " << j;
            }
        }
    }
}

TEST(Uuid7GeneratorTest, Generate_Unique)
{
    Uuid7Generator gen;
    std::set<std::string> ids;
    for (int i = 0; i < 10000; ++i)
    {
        ids.insert(gen.generate());
    }
    EXPECT_EQ(ids.size(), 10000u) << "All generated UUIDs must be unique";
}

TEST(Uuid7GeneratorTest, Generate_ThreadSafety)
{
    Uuid7Generator gen;
    std::vector<std::string> allIds;
    std::mutex mutex;
    std::vector<std::thread> threads;

    for (int t = 0; t < 8; ++t)
    {
        threads.emplace_back([&gen, &allIds, &mutex]() {
            std::vector<std::string> localIds;
            for (int i = 0; i < 1000; ++i)
            {
                localIds.push_back(gen.generate());
            }
            std::lock_guard<std::mutex> lock(mutex);
            allIds.insert(allIds.end(), localIds.begin(), localIds.end());
        });
    }

    for (auto& t : threads)
    {
        t.join();
    }

    std::set<std::string> uniqueIds(allIds.begin(), allIds.end());
    EXPECT_EQ(uniqueIds.size(), allIds.size()) << "Thread safety: all IDs must be unique";
}

TEST(Uuid7GeneratorTest, Generate_MonotonicTimestamps)
{
    Uuid7Generator gen;
    std::vector<std::string> ids;
    for (int i = 0; i < 100; ++i)
    {
        ids.push_back(gen.generate());
    }

    for (size_t i = 1; i < ids.size(); ++i)
    {
        auto ts1 = ids[i - 1].substr(0, 8) + ids[i - 1].substr(9, 4);
        auto ts2 = ids[i].substr(0, 8) + ids[i].substr(9, 4);
        EXPECT_LE(ts1, ts2) << "UUIDv7 timestamps should be monotonically non-decreasing";
    }
}

TEST(Uuid7GeneratorTest, Interface_Polymorphism)
{
    std::shared_ptr<IIdGenerator> gen = std::make_shared<Uuid7Generator>();
    std::string id = gen->generate();
    EXPECT_EQ(id.size(), 36u);
    EXPECT_EQ(id[14], '7');
}

TEST(Uuid7GeneratorTest, Generate_LowercaseOutput)
{
    Uuid7Generator gen;
    for (int i = 0; i < 100; ++i)
    {
        auto id = gen.generate();
        for (size_t j = 0; j < id.size(); ++j)
        {
            if (j != 8 && j != 13 && j != 18 && j != 23)
            {
                EXPECT_TRUE(std::islower(static_cast<unsigned char>(id[j])) || std::isdigit(static_cast<unsigned char>(id[j])))
                    << "Hex chars must be lowercase, got: " << id[j] << " at position " << j;
            }
        }
    }
}

TEST(UuidGeneratorTest, Legacy_UuidGenerator_StillWorks)
{
    UuidGenerator gen;
    auto id = gen.generate();
    EXPECT_EQ(id.size(), 32u);
    EXPECT_FALSE(id.empty());
}