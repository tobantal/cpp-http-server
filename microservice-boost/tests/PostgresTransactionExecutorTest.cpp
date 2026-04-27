#include <gtest/gtest.h>
#include <pqxx/pqxx>

#include "adapters/secondary/PostgresTransactionExecutor.hpp"
#include "adapters/secondary/TestLogger.hpp"
#include "adapters/secondary/ConnectionPool.hpp"

static std::string getTestConnString()
{
    const char* env = std::getenv("TEST_DB_CONNECTION");
    if (env && std::string(env).length() > 0) {
        return env;
    }
    return "host=localhost port=5432 dbname=postgres user=postgres password=postgres";
}

class PostgresTransactionExecutorTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        connString_ = getTestConnString();
        logger_ = std::make_shared<TestLogger>();

        try {
            pool_ = std::make_shared<ConnectionPool>(connString_,
                ConnectionPool::Config{1, 2}, logger_);
        } catch (const std::exception& e) {
            GTEST_SKIP() << "PostgreSQL not available: " << e.what();
        }

        executor_ = std::make_shared<PostgresTransactionExecutor>(logger_);

        auto conn = pool_->connection();
        pqxx::work txn(conn.get());
        txn.exec("DROP TABLE IF EXISTS txn_test");
        txn.exec("CREATE TABLE txn_test (id SERIAL PRIMARY KEY, name TEXT NOT NULL)");
        txn.commit();
    }

    void TearDown() override
    {
        if (!pool_) return;

        try {
            auto conn = pool_->connection();
            pqxx::work txn(conn.get());
            txn.exec("DROP TABLE IF EXISTS txn_test");
            txn.commit();
        } catch (...) {}
    }

    std::string connString_;
    std::shared_ptr<TestLogger> logger_;
    std::shared_ptr<ConnectionPool> pool_;
    std::shared_ptr<PostgresTransactionExecutor> executor_;
};

TEST_F(PostgresTransactionExecutorTest, ExecuteInsertsRow)
{
    auto conn = pool_->connection();

    executor_->execute(conn.get(), "test::insert",
        [](pqxx::work& txn) {
            txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("alice"));
        });

    auto result = executor_->query<int>(conn.get(), "test::count",
        [](pqxx::work& txn) -> int {
            auto r = txn.exec("SELECT COUNT(*) FROM txn_test");
            return r[0][0].as<int>();
        });

    EXPECT_EQ(result, 1);
}

TEST_F(PostgresTransactionExecutorTest, QueryReturnsCorrectData)
{
    auto conn = pool_->connection();

    executor_->execute(conn.get(), "test::insert",
        [](pqxx::work& txn) {
            txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("bob"));
            txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("carol"));
        });

    auto names = executor_->query<std::vector<std::string>>(conn.get(), "test::select",
        [](pqxx::work& txn) -> std::vector<std::string> {
            pqxx::result r = txn.exec("SELECT name FROM txn_test ORDER BY name");
            std::vector<std::string> result;
            for (auto row : r) {
                result.push_back(row[0].as<std::string>());
            }
            return result;
        });

    ASSERT_EQ(names.size(), 2u);
    EXPECT_EQ(names[0], "bob");
    EXPECT_EQ(names[1], "carol");
}

TEST_F(PostgresTransactionExecutorTest, QueryReturnsOptional)
{
    auto conn = pool_->connection();

    executor_->execute(conn.get(), "test::insert",
        [](pqxx::work& txn) {
            txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("dave"));
        });

    auto found = executor_->query<std::optional<std::string>>(conn.get(), "test::find",
        [](pqxx::work& txn) -> std::optional<std::string> {
            pqxx::result r = txn.exec_params(
                "SELECT name FROM txn_test WHERE name = $1", std::string("dave"));
            if (r.empty()) return std::nullopt;
            return r[0][0].as<std::string>();
        });

    ASSERT_TRUE(found.has_value());
    EXPECT_EQ(found.value(), "dave");

    auto notFound = executor_->query<std::optional<std::string>>(conn.get(), "test::find",
        [](pqxx::work& txn) -> std::optional<std::string> {
            pqxx::result r = txn.exec_params(
                "SELECT name FROM txn_test WHERE name = $1", std::string("nonexistent"));
            if (r.empty()) return std::nullopt;
            return r[0][0].as<std::string>();
        });

    EXPECT_FALSE(notFound.has_value());
}

TEST_F(PostgresTransactionExecutorTest, ExecuteRollobacksOnException)
{
    auto conn = pool_->connection();

    EXPECT_THROW(
        executor_->execute(conn.get(), "test::bad_insert",
            [](pqxx::work& txn) {
                txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("eve"));
                throw std::runtime_error("simulated failure");
            }),
        std::runtime_error);

    auto count = executor_->query<int>(conn.get(), "test::count",
        [](pqxx::work& txn) -> int {
            auto r = txn.exec("SELECT COUNT(*) FROM txn_test");
            return r[0][0].as<int>();
        });

    EXPECT_EQ(count, 0);
}

TEST_F(PostgresTransactionExecutorTest, QueryRollobacksOnException)
{
    auto conn = pool_->connection();

    EXPECT_THROW(
        executor_->query<int>(conn.get(), "test::bad_query",
            [](pqxx::work& txn) -> int {
                txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("frank"));
                throw std::logic_error("query failure");
            }),
        std::logic_error);

    auto count = executor_->query<int>(conn.get(), "test::count",
        [](pqxx::work& txn) -> int {
            auto r = txn.exec("SELECT COUNT(*) FROM txn_test");
            return r[0][0].as<int>();
        });

    EXPECT_EQ(count, 0);
}

TEST_F(PostgresTransactionExecutorTest, ErrorIsLoggedOnFailure)
{
    auto conn = pool_->connection();

    try {
        executor_->execute(conn.get(), "TestCtx::fail",
            [](pqxx::work&) {
                throw std::runtime_error("boom");
            });
    } catch (...) {}

    auto entries = logger_->getEntries();
    ASSERT_FALSE(entries.empty());

    bool foundError = false;
    for (const auto& entry : entries) {
        if (entry.level == LogLevel::Error &&
            entry.message.find("[TestCtx::fail]") != std::string::npos &&
            entry.message.find("boom") != std::string::npos) {
            foundError = true;
            break;
        }
    }
    EXPECT_TRUE(foundError);
}

TEST_F(PostgresTransactionExecutorTest, NullLoggerDoesNotCrash)
{
    PostgresTransactionExecutor nullExecutor;

    auto conn = pool_->connection();

    nullExecutor.execute(conn.get(), "test::null_log",
        [](pqxx::work& txn) {
            txn.exec_params("INSERT INTO txn_test(name) VALUES($1)", std::string("ghost"));
        });

    auto count = nullExecutor.query<int>(conn.get(), "test::count",
        [](pqxx::work& txn) -> int {
            auto r = txn.exec("SELECT COUNT(*) FROM txn_test");
            return r[0][0].as<int>();
        });

    EXPECT_EQ(count, 1);
}

TEST_F(PostgresTransactionExecutorTest, MultipleOperationsInSequence)
{
    auto conn = pool_->connection();

    for (int i = 0; i < 5; ++i) {
        executor_->execute(conn.get(), "test::insert_batch",
            [i](pqxx::work& txn) {
                txn.exec_params("INSERT INTO txn_test(name) VALUES($1)",
                    std::string("user_") + std::to_string(i));
            });
    }

    auto count = executor_->query<int>(conn.get(), "test::count",
        [](pqxx::work& txn) -> int {
            auto r = txn.exec("SELECT COUNT(*) FROM txn_test");
            return r[0][0].as<int>();
        });

    EXPECT_EQ(count, 5);
}
