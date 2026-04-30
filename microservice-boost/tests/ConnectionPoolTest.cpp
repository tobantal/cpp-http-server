#include <gtest/gtest.h>
#include <pqxx/pqxx>

#include "adapters/secondary/ConnectionPool.hpp"
#include "adapters/secondary/TestLogger.hpp"
#include <thread>
#include <vector>
#include <atomic>

static std::string getTestConnString()
{
    const char *env = std::getenv("TEST_DB_CONNECTION");
    if (env && std::string(env).length() > 0)
    {
        return env;
    }
    return "host=localhost port=5432 dbname=postgres user=postgres password=postgres";
}

class ConnectionPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        connString_ = getTestConnString();
        logger_ = std::make_shared<TestLogger>();

        try
        {
            pool_ = std::make_shared<ConnectionPool>(connString_,
                ConnectionPool::Config{2, 5}, logger_);
        }
        catch (const std::exception &e)
        {
            GTEST_SKIP() << "PostgreSQL not available: " << e.what();
        }
    }

    std::string connString_;
    std::shared_ptr<TestLogger> logger_;
    std::shared_ptr<ConnectionPool> pool_;
};

TEST_F(ConnectionPoolTest, InitialPoolHasMinConnections)
{
    EXPECT_GE(pool_->size(), 2u);
    EXPECT_GE(pool_->available(), 2u);
}

TEST_F(ConnectionPoolTest, ConnectionIsValid)
{
    auto conn = pool_->connection();
    EXPECT_TRUE(conn.valid());
    EXPECT_TRUE(conn.get().is_open());
}

TEST_F(ConnectionPoolTest, ConnectionReturnsToPool)
{
    EXPECT_GE(pool_->available(), 2u);
    size_t availableBefore = pool_->available();

    {
        auto conn = pool_->connection();
        EXPECT_EQ(pool_->available(), availableBefore - 1);
    }

    EXPECT_EQ(pool_->available(), availableBefore);
}

TEST_F(ConnectionPoolTest, ConnectionExecutesQuery)
{
    auto conn = pool_->connection();
    pqxx::work txn(conn.get());
    pqxx::result r = txn.exec("SELECT 1");
    EXPECT_EQ(r.size(), 1u);
    txn.commit();
}

TEST_F(ConnectionPoolTest, IsAliveReturnsTrue)
{
    EXPECT_TRUE(pool_->isAlive());
}

TEST_F(ConnectionPoolTest, ShutdownPreventsNewConnections)
{
    pool_->shutdown();
    EXPECT_EQ(pool_->available(), 0u);
}

TEST_F(ConnectionPoolTest, NameReturnsConnectionPool)
{
    EXPECT_EQ(pool_->name(), "ConnectionPool");
}

TEST_F(ConnectionPoolTest, PooledConnectionMoveSemantic)
{
    auto conn1 = pool_->connection();
    EXPECT_TRUE(conn1.valid());

    auto conn2 = std::move(conn1);
    EXPECT_TRUE(conn2.valid());
    EXPECT_FALSE(conn1.valid());
}

TEST_F(ConnectionPoolTest, ConcurrentCheckout)
{
    std::atomic<int> successCount{0};
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back([&]() {
            auto conn = pool_->connection();
            if (conn.valid())
            {
                pqxx::work txn(conn.get());
                txn.exec("SELECT 1");
                txn.commit();
                successCount++;
            }
        });
    }

    for (auto &t : threads)
    {
        t.join();
    }

    EXPECT_EQ(successCount, 10);
}

TEST_F(ConnectionPoolTest, DefaultConfigValues)
{
    EXPECT_EQ(ConnectionPool::defaultConfig.minConnections, 2u);
    EXPECT_EQ(ConnectionPool::defaultConfig.maxConnections, 10u);
}