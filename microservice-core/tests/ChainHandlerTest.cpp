#include <gtest/gtest.h>
#include "ChainHandler.hpp"
#include "SimpleRequest.hpp"
#include "SimpleResponse.hpp"
#include "TestLogger.hpp"
#include "NotFoundError.hpp"
#include "UnauthorizedError.hpp"
#include "BadRequestError.hpp"
#include "InternalError.hpp"

class OkHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        res.setResult(200, "application/json", R"({"status":"ok"})");
    }
};

class CreatedHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        res.setResult(201, "application/json", R"({"id":"new"})");
    }
};

class ThrowingHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        throw NotFoundError("User not found");
    }
};

class UnauthorizedThrowHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        throw UnauthorizedError("Invalid token");
    }
};

class StdExceptionHandler : public IHttpHandler
{
public:
    void handle(IRequest &req, IResponse &res) override
    {
        throw std::runtime_error("unexpected failure");
    }
};

class ChainHandlerTest : public ::testing::Test
{
protected:
    SimpleRequest req{"GET", "/test", "", "127.0.0.1", 80};
    SimpleResponse res;
};

TEST_F(ChainHandlerTest, SingleHandler_CompletesNormally)
{
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(ChainHandlerTest, SingleHandler_Sets201Status)
{
    auto handler = std::make_shared<CreatedHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 201);
}

TEST_F(ChainHandlerTest, MultipleHandlers_AllComplete)
{
    auto h1 = std::make_shared<OkHandler>();
    auto h2 = std::make_shared<OkHandler>();
    ChainHandler chain(h1, h2);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
}

TEST_F(ChainHandlerTest, HandlerThrowsNotFoundError_Returns404)
{
    auto handler = std::make_shared<ThrowingHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), R"({"error": "User not found"})");
}

TEST_F(ChainHandlerTest, HandlerThrowsUnauthorizedError_Returns401)
{
    auto handler = std::make_shared<UnauthorizedThrowHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 401);
    EXPECT_EQ(res.getBody(), R"({"error": "Invalid token"})");
}

TEST_F(ChainHandlerTest, HandlerThrowsStdException_Returns500)
{
    auto handler = std::make_shared<StdExceptionHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 500);
    EXPECT_EQ(res.getBody(), R"({"error": "Internal server error"})");
}

TEST_F(ChainHandlerTest, MiddlewareThrows_StopsChain)
{
    auto auth = std::make_shared<UnauthorizedThrowHandler>();
    auto ok = std::make_shared<OkHandler>();
    ChainHandler chain(auth, ok);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 401);
}

TEST_F(ChainHandlerTest, EmptyChain_DefaultStatus200)
{
    ChainHandler chain;
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
}

class InvalidStatusHandler : public IHttpHandler
{
public:
    void handle(IRequest &, IResponse &res) override
    {
        res.setStatus(50000);
    }
};

TEST_F(ChainHandlerTest, InvalidStatus_Returns500)
{
    auto handler = std::make_shared<InvalidStatusHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 500);
}

class JsonInjectionHandler : public IHttpHandler
{
public:
    void handle(IRequest &, IResponse &res) override
    {
        throw NotFoundError(R"(ok","injected":"true)");
    }
};

TEST_F(ChainHandlerTest, HttpErrorMessageWithQuotes_EscapesJson)
{
    auto handler = std::make_shared<JsonInjectionHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_EQ(res.getBody(), R"({"error": "ok\",\"injected\":\"true"})");
}

class StdExceptionInjectionHandler : public IHttpHandler
{
public:
    void handle(IRequest &, IResponse &res) override
    {
        throw std::runtime_error(R"(path\not\found)");
    }
};

TEST_F(ChainHandlerTest, StdExceptionMessage_NotExposedInResponse)
{
    auto handler = std::make_shared<StdExceptionInjectionHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 500);
    EXPECT_EQ(res.getBody(), R"({"error": "Internal server error"})");
}

// --- ChainHandler: X-Trace-ID ---

TEST_F(ChainHandlerTest, TraceId_GeneratesUuidWhenNoHeader)
{
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    auto traceHeader = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceHeader.has_value());
    EXPECT_EQ(traceHeader->size(), 32u);
}

TEST_F(ChainHandlerTest, TraceId_UsesExistingHeader)
{
    req.setHeader("X-Trace-ID", "my-custom-trace-id");
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    auto traceHeader = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceHeader.has_value());
    EXPECT_EQ(traceHeader.value(), "my-custom-trace-id");
}

TEST_F(ChainHandlerTest, TraceId_GetTraceIdReturnsSameValue)
{
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    std::string first = req.getTraceId();
    std::string second = req.getTraceId();
    EXPECT_EQ(first, second);
}

TEST_F(ChainHandlerTest, TraceId_SetTraceIdOverrides)
{
    req.setTraceId("overridden-id");
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    auto traceHeader = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceHeader.has_value());
    EXPECT_EQ(traceHeader.value(), "overridden-id");
}

TEST_F(ChainHandlerTest, TraceId_IncludedOnError)
{
    req.setHeader("X-Trace-ID", "error-trace-123");
    auto handler = std::make_shared<ThrowingHandler>();
    ChainHandler chain(handler);
    chain.handle(req, res);

    auto traceHeader = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceHeader.has_value());
    EXPECT_EQ(traceHeader.value(), "error-trace-123");
}

// --- ChainHandler: logging with TestLogger ---

TEST_F(ChainHandlerTest, DebugLog_TraceIdInSuccessPath)
{
    auto logger = std::make_shared<TestLogger>();
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(std::static_pointer_cast<ILogger>(logger), handler);
    chain.handle(req, res);

    std::string traceId = req.getTraceId();
    ASSERT_GE(logger->size(), 2u);
    EXPECT_EQ(logger->at(0).level, LogLevel::Debug);
    EXPECT_EQ(logger->at(0).category, "ChainHandler");
    EXPECT_EQ(logger->at(0).message, "[" + traceId + "] Handler started");
    EXPECT_EQ(logger->at(1).level, LogLevel::Debug);
    EXPECT_EQ(logger->at(1).category, "ChainHandler");
    EXPECT_EQ(logger->at(1).message, "[" + traceId + "] Handler finished with status 200");
}

TEST_F(ChainHandlerTest, DebugLog_MultipleHandlers)
{
    auto logger = std::make_shared<TestLogger>();
    auto h1 = std::make_shared<OkHandler>();
    auto h2 = std::make_shared<OkHandler>();
    ChainHandler chain(std::static_pointer_cast<ILogger>(logger), h1, h2);
    chain.handle(req, res);

    std::string traceId = req.getTraceId();
    ASSERT_GE(logger->size(), 4u);
    EXPECT_EQ(logger->at(0).message, "[" + traceId + "] Handler started");
    EXPECT_EQ(logger->at(1).message, "[" + traceId + "] Handler finished with status 200");
    EXPECT_EQ(logger->at(2).message, "[" + traceId + "] Handler started");
    EXPECT_EQ(logger->at(3).message, "[" + traceId + "] Handler finished with status 200");
}

TEST_F(ChainHandlerTest, ErrorLog_TraceIdInHttpError)
{
    auto logger = std::make_shared<TestLogger>();
    req.setHeader("X-Trace-ID", "err-001");
    auto handler = std::make_shared<ThrowingHandler>();
    ChainHandler chain(std::static_pointer_cast<ILogger>(logger), handler);
    chain.handle(req, res);

    ASSERT_GE(logger->size(), 2u);
    EXPECT_EQ(logger->at(0).level, LogLevel::Debug);
    EXPECT_EQ(logger->at(0).message, "[err-001] Handler started");
    EXPECT_EQ(logger->at(1).level, LogLevel::Error);
    EXPECT_EQ(logger->at(1).message, "[err-001] HttpError: 404 - User not found");
}

TEST_F(ChainHandlerTest, ErrorLog_TraceIdInStdException)
{
    auto logger = std::make_shared<TestLogger>();
    req.setHeader("X-Trace-ID", "exc-002");
    auto handler = std::make_shared<StdExceptionHandler>();
    ChainHandler chain(std::static_pointer_cast<ILogger>(logger), handler);
    chain.handle(req, res);

    ASSERT_GE(logger->size(), 2u);
    EXPECT_EQ(logger->at(0).level, LogLevel::Debug);
    EXPECT_EQ(logger->at(0).message, "[exc-002] Handler started");
    EXPECT_EQ(logger->at(1).level, LogLevel::Error);
    EXPECT_EQ(logger->at(1).category, "ChainHandler");
}

TEST_F(ChainHandlerTest, DebugLog_UsesExistingTraceId)
{
    auto logger = std::make_shared<TestLogger>();
    req.setHeader("X-Trace-ID", "incoming-trace-42");
    auto handler = std::make_shared<OkHandler>();
    ChainHandler chain(std::static_pointer_cast<ILogger>(logger), handler);
    chain.handle(req, res);

    ASSERT_GE(logger->size(), 2u);
    EXPECT_TRUE(logger->at(0).message.find("[incoming-trace-42]") != std::string::npos);
    EXPECT_TRUE(logger->at(1).message.find("[incoming-trace-42]") != std::string::npos);
}