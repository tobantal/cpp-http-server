#include <gtest/gtest.h>
#include "adapters/primary/SchemaValidator.hpp"
#include "adapters/primary/JsonProcessor.hpp"
#include "adapters/primary/JsonToEnvConverter.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"
#include "adapters/secondary/Environment.hpp"
#include "domain/schema/Schema.hpp"
#include "domain/error/BadRequestError.hpp"
#include "domain/error/ConvertError.hpp"
#include "domain/error/HttpError.hpp"
#include "ports/input/IJsonProcessor.hpp"
#include "ports/output/IEnvironment.hpp"
#include <memory>
#include <string>

class OrderSchema : public Schema
{
public:
    OrderSchema()
    {
        field<std::string>("symbol").required().minLength(1).maxLength(10);
        field<int>("amount").required().min(1).max(10000);
        field<double>("price").optional().max(1000000);
        field<bool>("active").optional();
    }
};

class LoginSchema : public Schema
{
public:
    LoginSchema()
    {
        field<std::string>("username").required().minLength(3).maxLength(50);
        field<std::string>("password").required().minLength(8);
    }
};

static std::shared_ptr<IJsonToEnvConverter> makeConverter()
{
    return std::make_shared<JsonToEnvConverter>();
}

static SimpleRequest makeJsonRequest(const std::string& body)
{
    return SimpleRequest("POST", "/test", body, "127.0.0.1", 80,
                         {{"Content-Type", "application/json"}});
}

class SchemaValidatorTest : public ::testing::Test
{
protected:
    std::shared_ptr<IJsonToEnvConverter> converter_{makeConverter()};
    std::shared_ptr<JsonProcessor> processor_{std::make_shared<JsonProcessor>(converter_)};
    SimpleResponse dummy_;
};

TEST_F(SchemaValidatorTest, AllRequiredFieldsPresent_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":5})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, AllFieldsWithOptional_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"ETH","amount":10,"price":1234.56,"active":true})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, MissingRequiredField_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC"})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("amount is required"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, MultipleMissingRequiredFields_ReportsAll)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        std::string msg = e.message();
        EXPECT_NE(msg.find("symbol is required"), std::string::npos);
        EXPECT_NE(msg.find("amount is required"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, OptionalFieldAbsent_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"ETH","amount":10})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, WrongTypeInt_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":"not_a_number"})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("amount must be an integer"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, WrongTypeString_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":12345,"amount":5})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("symbol must be a string"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, WrongTypeBool_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":5,"active":"yes"})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("active must be a boolean"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, IntBelowMin_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":0})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("amount must be >= 1"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, IntAboveMax_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":20000})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("amount must be <= 10000"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, DoubleAboveMax_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":5,"price":2000000.0})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("price must be <= 1000000"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, StringBelowMinLength_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<LoginSchema>());
    SimpleRequest req = makeJsonRequest(R"({"username":"ab","password":"longpassword"})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("username minLength is 3"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, StringAboveMaxLength_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<LoginSchema>());
    SimpleRequest req = makeJsonRequest(R"({"username":"areallylongusernamethatiswayoverfiftycharacterslong","password":"longpassword"})");
    processor_->handle(req, dummy_);
    try
    {
        validator->handle(req, dummy_);
        FAIL() << "Expected BadRequestError";
    }
    catch (const BadRequestError& e)
    {
        EXPECT_NE(std::string(e.message()).find("username maxLength is 50"), std::string::npos);
    }
}

TEST_F(SchemaValidatorTest, NoJsonObject_Throws)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req;
    EXPECT_THROW(validator->handle(req, dummy_), BadRequestError);
}

TEST_F(SchemaValidatorTest, IntAtMinBoundary_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":1})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, IntAtMaxBoundary_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":10000})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, DoubleAtMaxBoundary_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    SimpleRequest req = makeJsonRequest(R"({"symbol":"BTC","amount":5,"price":1000000.0})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, StringAtMinLength_Passes)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<LoginSchema>());
    SimpleRequest req = makeJsonRequest(R"({"username":"abc","password":"longpassword"})");
    processor_->handle(req, dummy_);
    EXPECT_NO_THROW(validator->handle(req, dummy_));
}

TEST_F(SchemaValidatorTest, NameReturnsSchemaValidator)
{
    auto validator = std::make_shared<SchemaValidator>(std::make_shared<OrderSchema>());
    EXPECT_EQ(validator->name(), "SchemaValidator");
}