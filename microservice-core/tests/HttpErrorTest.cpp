#include <gtest/gtest.h>
#include "domain/error/HttpError.hpp"
#include "domain/error/BadRequestError.hpp"
#include "domain/error/UnauthorizedError.hpp"
#include "domain/error/ForbiddenError.hpp"
#include "domain/error/NotFoundError.hpp"
#include "domain/error/ConflictError.hpp"
#include "domain/error/InternalError.hpp"
#include "domain/error/ServiceUnavailableError.hpp"
#include "domain/error/BusinessError.hpp"
#include "domain/error/AuthError.hpp"
#include "domain/error/MethodNotAllowedError.hpp"

TEST(HttpErrorTest, BaseErrorStatusCode)
{
    HttpError e(418, "I'm a teapot");
    EXPECT_EQ(e.statusCode(), 418);
    EXPECT_EQ(e.message(), "I'm a teapot");
    EXPECT_STREQ(e.what(), "I'm a teapot");
}

TEST(HttpErrorTest, BadRequestDefault)
{
    BadRequestError e;
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Bad request");
}

TEST(HttpErrorTest, BadRequestCustomMessage)
{
    BadRequestError e("Invalid email format");
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Invalid email format");
}

TEST(HttpErrorTest, UnauthorizedDefault)
{
    UnauthorizedError e;
    EXPECT_EQ(e.statusCode(), 401);
    EXPECT_EQ(e.message(), "Unauthorized");
}

TEST(HttpErrorTest, UnauthorizedCustomMessage)
{
    UnauthorizedError e("Token expired");
    EXPECT_EQ(e.statusCode(), 401);
    EXPECT_EQ(e.message(), "Token expired");
}

TEST(HttpErrorTest, ForbiddenDefault)
{
    ForbiddenError e;
    EXPECT_EQ(e.statusCode(), 403);
}

TEST(HttpErrorTest, NotFoundDefault)
{
    NotFoundError e;
    EXPECT_EQ(e.statusCode(), 404);
    EXPECT_EQ(e.message(), "Not found");
}

TEST(HttpErrorTest, NotFoundCustomMessage)
{
    NotFoundError e("Order not found");
    EXPECT_EQ(e.statusCode(), 404);
    EXPECT_EQ(e.message(), "Order not found");
}

TEST(HttpErrorTest, ConflictDefault)
{
    ConflictError e;
    EXPECT_EQ(e.statusCode(), 409);
}

TEST(HttpErrorTest, MethodNotAllowedDefault)
{
    MethodNotAllowedError e;
    EXPECT_EQ(e.statusCode(), 405);
    EXPECT_EQ(e.message(), "Method not allowed");
}

TEST(HttpErrorTest, MethodNotAllowedCustomMessage)
{
    MethodNotAllowedError e("POST not allowed for /api/orders");
    EXPECT_EQ(e.statusCode(), 405);
    EXPECT_EQ(e.message(), "POST not allowed for /api/orders");
}

TEST(HttpErrorTest, InternalErrorDefault)
{
    InternalError e;
    EXPECT_EQ(e.statusCode(), 500);
}

TEST(HttpErrorTest, InternalErrorCustomMessage)
{
    InternalError e("Database connection failed");
    EXPECT_EQ(e.statusCode(), 500);
    EXPECT_EQ(e.message(), "Database connection failed");
}

TEST(HttpErrorTest, ServiceUnavailableDefault)
{
    ServiceUnavailableError e;
    EXPECT_EQ(e.statusCode(), 503);
}

TEST(HttpErrorTest, BusinessErrorDefault)
{
    BusinessError e;
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Business error");
}

TEST(HttpErrorTest, BusinessErrorCustomMessage)
{
    BusinessError e("Insufficient funds");
    EXPECT_EQ(e.statusCode(), 400);
    EXPECT_EQ(e.message(), "Insufficient funds");
}

TEST(HttpErrorTest, AuthErrorDefault)
{
    AuthError e;
    EXPECT_EQ(e.statusCode(), 401);
    EXPECT_EQ(e.message(), "Authentication error");
}

TEST(HttpErrorTest, AuthErrorCustomMessage)
{
    AuthError e("Invalid credentials");
    EXPECT_EQ(e.statusCode(), 401);
    EXPECT_EQ(e.message(), "Invalid credentials");
}

TEST(HttpErrorTest, CatchByHttpErrorRef)
{
    try
    {
        throw NotFoundError("User not found");
    }
    catch (const HttpError &e)
    {
        EXPECT_EQ(e.statusCode(), 404);
        EXPECT_EQ(e.message(), "User not found");
    }
}

TEST(HttpErrorTest, CatchByStdException)
{
    try
    {
        throw InternalError("Something broke");
    }
    catch (const std::exception &e)
    {
        EXPECT_STREQ(e.what(), "Something broke");
    }
}
