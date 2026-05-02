#include <gtest/gtest.h>
#include "application/BaseWebApplication.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"
#include "adapters/secondary/TestLogger.hpp"
#include "domain/error/NotFoundError.hpp"
#include "domain/error/BadRequestError.hpp"
#include "domain/error/UnauthorizedError.hpp"
#include "domain/error/MethodNotAllowedError.hpp"

class TestableApp : public BaseWebApplication
{
public:
    TestableApp() : BaseWebApplication(std::make_shared<TestLogger>()) {}

    void loadEnvironment(int, char *[]) override {}
    void configureInjection() override {}
    void start() override {}
    std::string name() const override { return "TestableApp"; }
    using BaseWebApplication::registerHandler;
    using BaseWebApplication::handleRequest;
    using BaseWebApplication::state_;
};

namespace
{
    class OkHandler : public IHttpHandler
    {
    public:
        std::string name() const override { return "OkHandler"; }
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(200, "application/json", R"({"status":"ok"})");
        }
    };

    class CreatedHandler : public IHttpHandler
    {
    public:
        std::string name() const override { return "CreatedHandler"; }
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(201, "application/json", R"({"id":"new"})");
        }
    };

    class ThrowingHandler : public IHttpHandler
    {
    public:
        std::string name() const override { return "ThrowingHandler"; }
        void handle(IRequest &, IResponse &) override
        {
            throw NotFoundError("User not found");
        }
    };

    class BadRequestThrowHandler : public IHttpHandler
    {
    public:
        std::string name() const override { return "BadRequestThrowHandler"; }
        void handle(IRequest &, IResponse &) override
        {
            throw BadRequestError("Invalid input");
        }
    };

    class StdExceptionHandler : public IHttpHandler
    {
    public:
        std::string name() const override { return "StdExceptionHandler"; }
        void handle(IRequest &, IResponse &) override
        {
            throw std::runtime_error("unexpected failure");
        }
    };
}

class BaseWebApplicationTest : public ::testing::Test
{
protected:
    TestableApp app;
};

TEST_F(BaseWebApplicationTest, RegisterHandlerAllowedBeforeStart)
{
    EXPECT_NO_THROW(app.registerHandler("GET", "/test", std::make_shared<OkHandler>()));
}

TEST_F(BaseWebApplicationTest, RegisterHandlerMultipleAllowedBeforeStart)
{
    EXPECT_NO_THROW(app.registerHandler("GET", "/a", std::make_shared<OkHandler>()));
    EXPECT_NO_THROW(app.registerHandler("POST", "/b", std::make_shared<OkHandler>()));
    EXPECT_NO_THROW(app.registerHandler("DELETE", "/c/*", std::make_shared<OkHandler>()));
}

TEST_F(BaseWebApplicationTest, RegisterEndpoint_AllowedBeforeStart)
{
    EXPECT_NO_THROW(app.registerEndpoint("GET", "/ep", std::make_shared<OkHandler>()));
}

TEST_F(BaseWebApplicationTest, RegisterHandlerBlockedAfterStart)
{
    app.state_.store(ServerState::Running);
    EXPECT_THROW(app.registerHandler("GET", "/late", std::make_shared<OkHandler>()),
                 std::logic_error);
}

TEST_F(BaseWebApplicationTest, StopWithoutStartIsNoop)
{
    EXPECT_NO_THROW(app.stop());
}

TEST_F(BaseWebApplicationTest, DoubleStopWithoutStartIsNoop)
{
    EXPECT_NO_THROW(app.stop());
    EXPECT_NO_THROW(app.stop());
}

TEST_F(BaseWebApplicationTest, ServerStateEnumValues)
{
    EXPECT_NE(ServerState::NotStarted, ServerState::Running);
    EXPECT_NE(ServerState::Running, ServerState::Stopped);
    EXPECT_NE(ServerState::NotStarted, ServerState::Stopped);
}

TEST_F(BaseWebApplicationTest, HandleRequest_ExactMatch)
{
    app.registerHandler("GET", "/hello", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/hello", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_DifferentMethodsOnSamePath)
{
    app.registerHandler("GET", "/items", std::make_shared<OkHandler>());
    app.registerHandler("POST", "/items", std::make_shared<CreatedHandler>());

    SimpleRequest getReq{"GET", "/items", "", "127.0.0.1", 80};
    SimpleResponse getRes;
    app.handleRequest(getReq, getRes);
    EXPECT_EQ(getRes.getStatus(), 200);

    SimpleRequest postReq{"POST", "/items", "", "127.0.0.1", 80};
    SimpleResponse postRes;
    app.handleRequest(postReq, postRes);
    EXPECT_EQ(postRes.getStatus(), 201);
}

TEST_F(BaseWebApplicationTest, HandleRequest_WildcardMatch)
{
    app.registerHandler("GET", "/users/*", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/users/123", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_WildcardSetsPathPattern)
{
    app.registerHandler("GET", "/items/*", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/items/42", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(req.getPathPattern(), "/items/*");
}

TEST_F(BaseWebApplicationTest, HandleRequest_404NotFound)
{
    SimpleRequest req{"GET", "/nonexistent", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), R"({"error": "Not found"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_405WithAllowHeader)
{
    app.registerHandler("GET", "/items", std::make_shared<OkHandler>());
    app.registerHandler("POST", "/items", std::make_shared<CreatedHandler>());

    SimpleRequest req{"DELETE", "/items", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 405);
    auto allow = res.getHeader("Allow");
    ASSERT_TRUE(allow.has_value());
    EXPECT_TRUE(allow->find("GET") != std::string::npos);
    EXPECT_TRUE(allow->find("POST") != std::string::npos);
}

TEST_F(BaseWebApplicationTest, HandleRequest_405WildcardPath)
{
    app.registerHandler("GET", "/users/*", std::make_shared<OkHandler>());

    SimpleRequest req{"POST", "/users/123", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 405);
    auto allow = res.getHeader("Allow");
    ASSERT_TRUE(allow.has_value());
    EXPECT_TRUE(allow->find("GET") != std::string::npos);
}

TEST_F(BaseWebApplicationTest, HandleRequest_HttpError_CaughtAndReturned)
{
    app.registerHandler("GET", "/fail", std::make_shared<ThrowingHandler>());

    SimpleRequest req{"GET", "/fail", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), R"({"error": "User not found"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_HttpError400_CaughtAndReturned)
{
    app.registerHandler("POST", "/bad", std::make_shared<BadRequestThrowHandler>());

    SimpleRequest req{"POST", "/bad", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 400);
    EXPECT_EQ(res.getBody(), R"({"error": "Invalid input"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_StdException_Returns500)
{
    app.registerHandler("GET", "/crash", std::make_shared<StdExceptionHandler>());

    SimpleRequest req{"GET", "/crash", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 500);
    EXPECT_EQ(res.getBody(), R"({"error": "Internal server error"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_RegisterEndpointWithChainHandler)
{
    app.registerEndpoint("GET", "/chain", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/chain", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_ExactMatchPreferredOverWildcard)
{
    auto wildcardHandler = std::make_shared<CreatedHandler>();
    auto exactHandler = std::make_shared<OkHandler>();
    app.registerHandler("GET", "/users/*", wildcardHandler);
    app.registerHandler("GET", "/users/me", exactHandler);

    SimpleRequest req{"GET", "/users/me", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_LogsRequest)
{
    auto logger = std::make_shared<TestLogger>();
    TestableApp loggedApp;
    loggedApp.registerHandler("GET", "/logged", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/logged", "", "10.0.0.1", 80};
    SimpleResponse res;
    loggedApp.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
}

TEST_F(BaseWebApplicationTest, Shutdown_CallsStop)
{
    EXPECT_NO_THROW(app.shutdown());
}

TEST_F(BaseWebApplicationTest, Name_ReturnsOverriddenValue)
{
    EXPECT_EQ(app.name(), "TestableApp");
}

TEST_F(BaseWebApplicationTest, HandleRequest_NamedParamMatch)
{
    app.registerHandler("GET", "/users/:id", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/users/42", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(req.getPathPattern(), "/users/:id");
}

TEST_F(BaseWebApplicationTest, HandleRequest_NamedParamSetsPathPattern)
{
    app.registerHandler("GET", "/api/:version/users/:id", std::make_shared<OkHandler>());

    SimpleRequest req{"GET", "/api/v1/users/42", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(req.getPathPattern(), "/api/:version/users/:id");
    EXPECT_EQ(*req.getPathParam("version"), "v1");
    EXPECT_EQ(*req.getPathParam("id"), "42");
}

TEST_F(BaseWebApplicationTest, HandleRequest_StaticPreferredOverNamedParam)
{
    auto namedHandler = std::make_shared<CreatedHandler>();
    auto staticHandler = std::make_shared<OkHandler>();
    app.registerHandler("GET", "/users/:id", namedHandler);
    app.registerHandler("GET", "/users/me", staticHandler);

    SimpleRequest req{"GET", "/users/me", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_NamedParamPreferredOverWildcard)
{
    auto wildcardHandler = std::make_shared<CreatedHandler>();
    auto namedHandler = std::make_shared<OkHandler>();
    app.registerHandler("GET", "/users/*", wildcardHandler);
    app.registerHandler("GET", "/users/:id", namedHandler);

    SimpleRequest req{"GET", "/users/42", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(BaseWebApplicationTest, HandleRequest_405NamedParamPath)
{
    app.registerHandler("GET", "/users/:id", std::make_shared<OkHandler>());

    SimpleRequest req{"POST", "/users/123", "", "127.0.0.1", 80};
    SimpleResponse res;
    app.handleRequest(req, res);

    EXPECT_EQ(res.getStatus(), 405);
    auto allow = res.getHeader("Allow");
    ASSERT_TRUE(allow.has_value());
    EXPECT_TRUE(allow->find("GET") != std::string::npos);
}