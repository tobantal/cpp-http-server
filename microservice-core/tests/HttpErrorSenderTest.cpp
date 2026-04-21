#include <gtest/gtest.h>
#include "handler/HttpErrorSender.hpp"
#include "adapters/secondary/SimpleResponse.hpp"
#include "domain/error/HttpError.hpp"
#include "domain/error/NotFoundError.hpp"
#include "domain/error/BadRequestError.hpp"

/**
 * @file HttpErrorSenderTest.cpp
 * @brief Unit tests for HttpErrorSender
 */

TEST(HttpErrorSenderTest, HandlesNotFoundError)
{
    HttpErrorSender sender;
    SimpleResponse res;
    NotFoundError e("Page not found");

    sender.handleError(res, e);

    EXPECT_EQ(res.getStatus(), 404);
    EXPECT_NE(res.getBody().find("Page not found"), std::string::npos);
    EXPECT_NE(res.getBody().find(R"("error")"), std::string::npos);
}

TEST(HttpErrorSenderTest, HandlesBadRequestError)
{
    HttpErrorSender sender;
    SimpleResponse res;
    BadRequestError e("Invalid input");

    sender.handleError(res, e);

    EXPECT_EQ(res.getStatus(), 400);
    EXPECT_NE(res.getBody().find("Invalid input"), std::string::npos);
}

TEST(HttpErrorSenderTest, HandlesGenericHttpError)
{
    HttpErrorSender sender;
    SimpleResponse res;
    HttpError e(500, "Internal server error");

    sender.handleError(res, e);

    EXPECT_EQ(res.getStatus(), 500);
    EXPECT_NE(res.getBody().find("Internal server error"), std::string::npos);
}

TEST(HttpErrorSenderTest, EscapesJsonInMessage)
{
    HttpErrorSender sender;
    SimpleResponse res;
    HttpError e(400, R"(Value "test" has <tags>)");

    sender.handleError(res, e);

    EXPECT_EQ(res.getStatus(), 400);
    EXPECT_NE(res.getBody().find(R"(Value \"test\" has <tags>)"), std::string::npos);
}

TEST(HttpErrorSenderTest, SetsContentTypeToJson)
{
    HttpErrorSender sender;
    SimpleResponse res;
    HttpError e(403, "Forbidden");

    sender.handleError(res, e);

    auto ct = res.getHeader("Content-Type");
    ASSERT_TRUE(ct.has_value());
    EXPECT_NE(ct->find("application/json"), std::string::npos);
}

TEST(HttpErrorSenderTest, PreservesTraceId)
{
    HttpErrorSender sender;
    SimpleResponse res;
    res.setHeader("X-Trace-ID", "test-trace-123");
    HttpError e(500, "Error");

    sender.handleError(res, e);

    auto traceId = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceId.has_value());
    EXPECT_EQ(*traceId, "test-trace-123");
}

TEST(HttpErrorSenderTest, SetsTraceIdWhenPresent)
{
    HttpErrorSender sender;
    SimpleResponse res;
    res.setHeader("X-Trace-ID", "my-trace-id");
    HttpError e(404, "Not found");

    sender.handleError(res, e);

    auto traceId = res.getHeader("X-Trace-ID");
    ASSERT_TRUE(traceId.has_value());
    EXPECT_EQ(*traceId, "my-trace-id");
}