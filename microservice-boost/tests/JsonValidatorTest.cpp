#include <gtest/gtest.h>
#include "adapters/primary/handler/JsonValidator.hpp"
#include "application/ChainHandler.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"

class JsonValidatorTest : public ::testing::Test
{
protected:
    SimpleResponse res;
};

TEST_F(JsonValidatorTest, ValidJsonObject_PassesThrough)
{
    SimpleRequest req("POST", "/api/orders",
                      R"({"accountId": "acc-1", "quantity": 10})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_NO_THROW(validator.handle(req, res));
}

TEST_F(JsonValidatorTest, ValidJsonArray_PassesThrough)
{
    SimpleRequest req("POST", "/api/orders", "[1, 2, 3]",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_NO_THROW(validator.handle(req, res));
}

TEST_F(JsonValidatorTest, ValidJsonString_PassesThrough)
{
    SimpleRequest req("POST", "/api/orders", R"("hello")",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_NO_THROW(validator.handle(req, res));
}

TEST_F(JsonValidatorTest, EmptyObject_PassesThrough)
{
    SimpleRequest req("POST", "/api/orders", "{}",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_NO_THROW(validator.handle(req, res));
}

TEST_F(JsonValidatorTest, InvalidJson_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", "not json at all",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, TrailingCommaJson_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", R"({"key": "value",})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, MissingQuotesJson_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", R"({key: "value"})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, EmptyBody_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", "",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, NonJsonContentType_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", R"({"key": "value"})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "text/plain"}});
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, NoContentType_ThrowsBadRequestError)
{
    SimpleRequest req("POST", "/api/orders", R"({"key": "value"})",
                      "127.0.0.1", 80);
    JsonValidator validator;

    EXPECT_THROW(validator.handle(req, res), BadRequestError);
}

TEST_F(JsonValidatorTest, ContentTypeVariantJson_PassesThrough)
{
    SimpleRequest req("POST", "/api/orders", R"({"ok": true})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json; charset=utf-8"}});
    JsonValidator validator;

    EXPECT_NO_THROW(validator.handle(req, res));
}

TEST_F(JsonValidatorTest, InChainWithHandler_InvalidJsonStops)
{
    class OkHandler : public IHttpHandler
    {
    public:
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(200, "application/json", R"({"status":"ok"})");
        }
    };

    SimpleRequest req("POST", "/api/orders", "invalid",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    ChainHandler chain(std::make_shared<JsonValidator>(),
                       std::make_shared<OkHandler>());
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 400);
}

TEST_F(JsonValidatorTest, InChainWithHandler_ValidJsonContinues)
{
    class OkHandler : public IHttpHandler
    {
    public:
        void handle(IRequest &, IResponse &res) override
        {
            res.setResult(200, "application/json", R"({"status":"ok"})");
        }
    };

    SimpleRequest req("POST", "/api/orders", R"({"name":"test"})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    ChainHandler chain(std::make_shared<JsonValidator>(),
                       std::make_shared<OkHandler>());
    chain.handle(req, res);

    EXPECT_EQ(res.getStatus(), 200);
    EXPECT_EQ(res.getBody(), R"({"status":"ok"})");
}

TEST_F(JsonValidatorTest, ErrorMessageContainsInvalidJson)
{
    SimpleRequest req("POST", "/api/orders", "not json",
                      "127.0.0.1", 80,
                      {{"Content-Type", "application/json"}});
    JsonValidator validator;

    try
    {
        validator.handle(req, res);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError &e)
    {
        EXPECT_EQ(e.statusCode(), 400);
        EXPECT_NE(std::string(e.message()).find("Invalid JSON"), std::string::npos);
    }
}

TEST_F(JsonValidatorTest, ErrorMessageForNonJsonContentType)
{
    SimpleRequest req("POST", "/api/orders", R"({"key":"val"})",
                      "127.0.0.1", 80,
                      {{"Content-Type", "text/plain"}});
    JsonValidator validator;

    try
    {
        validator.handle(req, res);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError &e)
    {
        EXPECT_EQ(e.statusCode(), 400);
        EXPECT_NE(std::string(e.message()).find("application/json"), std::string::npos);
    }
}