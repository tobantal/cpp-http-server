#include <gtest/gtest.h>
#include "application/RouteTrie.hpp"
#include "adapters/secondary/SimpleResponse.hpp"

namespace
{
    class TestHandler : public IHttpHandler
    {
    public:
        explicit TestHandler(std::string n) : name_(std::move(n)) {}
        std::string name() const override { return name_; }
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(200, "text/plain", name_);
        }

    private:
        std::string name_;
    };
}

class RouteTrieTest : public ::testing::Test
{
protected:
    RouteTrie trie;
};

TEST_F(RouteTrieTest, ExactMatch)
{
    trie.insert("/api/users", "GET", std::make_shared<TestHandler>("users"));
    auto match = trie.lookup("GET", "/api/users");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pattern, "/api/users");
}

TEST_F(RouteTrieTest, NamedParam)
{
    trie.insert("/users/:id", "GET", std::make_shared<TestHandler>("userById"));
    auto match = trie.lookup("GET", "/users/42");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pattern, "/users/:id");
    EXPECT_EQ(match->pathParams.at("id"), "42");
}

TEST_F(RouteTrieTest, Wildcard)
{
    trie.insert("/files/*", "GET", std::make_shared<TestHandler>("files"));
    auto match = trie.lookup("GET", "/files/readme.txt");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pattern, "/files/*");
}

TEST_F(RouteTrieTest, Priority_StaticOverNamedParam)
{
    trie.insert("/users/:id", "GET", std::make_shared<TestHandler>("named"));
    trie.insert("/users/me", "GET", std::make_shared<TestHandler>("static"));

    auto match = trie.lookup("GET", "/users/me");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pattern, "/users/me");
}

TEST_F(RouteTrieTest, Priority_NamedParamOverWildcard)
{
    trie.insert("/users/*", "GET", std::make_shared<TestHandler>("wildcard"));
    trie.insert("/users/:id", "GET", std::make_shared<TestHandler>("named"));

    auto match = trie.lookup("GET", "/users/42");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pattern, "/users/:id");
    EXPECT_EQ(match->pathParams.at("id"), "42");
}

TEST_F(RouteTrieTest, NotFound)
{
    trie.insert("/api/users", "GET", std::make_shared<TestHandler>("users"));
    auto match = trie.lookup("GET", "/unknown");
    EXPECT_FALSE(match.has_value());
}

TEST_F(RouteTrieTest, MethodNotAllowed)
{
    trie.insert("/users", "GET", std::make_shared<TestHandler>("users"));

    auto match = trie.lookup("POST", "/users");
    EXPECT_FALSE(match.has_value());

    EXPECT_TRUE(trie.lookupAny("/users"));
}

TEST_F(RouteTrieTest, MultipleParams)
{
    trie.insert("/api/:version/users/:id", "GET", std::make_shared<TestHandler>("multi"));
    auto match = trie.lookup("GET", "/api/v1/users/42");
    ASSERT_TRUE(match.has_value());
    EXPECT_EQ(match->pathParams.at("version"), "v1");
    EXPECT_EQ(match->pathParams.at("id"), "42");
}

TEST_F(RouteTrieTest, SegmentCountMismatch)
{
    trie.insert("/users/:id", "GET", std::make_shared<TestHandler>("users"));
    auto match = trie.lookup("GET", "/users");
    EXPECT_FALSE(match.has_value());
}

TEST_F(RouteTrieTest, GetAllowedMethods)
{
    trie.insert("/items", "GET", std::make_shared<TestHandler>("get"));
    trie.insert("/items", "POST", std::make_shared<TestHandler>("post"));
    trie.insert("/items", "DELETE", std::make_shared<TestHandler>("delete"));

    auto methods = trie.lookupMethods("/items");
    EXPECT_EQ(methods.size(), 3u);
    EXPECT_NE(std::find(methods.begin(), methods.end(), "GET"), methods.end());
    EXPECT_NE(std::find(methods.begin(), methods.end(), "POST"), methods.end());
    EXPECT_NE(std::find(methods.begin(), methods.end(), "DELETE"), methods.end());
}

TEST_F(RouteTrieTest, LookupAnyReturnsTrue)
{
    trie.insert("/api/:id", "GET", std::make_shared<TestHandler>("api"));
    EXPECT_TRUE(trie.lookupAny("/api/123"));
    EXPECT_FALSE(trie.lookupAny("/other/123"));
}

TEST_F(RouteTrieTest, EmptyTrieReturnsNullopt)
{
    EXPECT_FALSE(trie.lookup("GET", "/anything").has_value());
    EXPECT_FALSE(trie.lookupAny("/anything"));
    EXPECT_TRUE(trie.lookupMethods("/anything").empty());
}
