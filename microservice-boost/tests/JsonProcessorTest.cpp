#include <gtest/gtest.h>
#include "adapters/primary/JsonProcessor.hpp"
#include "adapters/primary/JsonToEnvConverter.hpp"
#include "adapters/secondary/SimpleRequest.hpp"
#include "adapters/secondary/SimpleResponse.hpp"
#include "domain/error/BadRequestError.hpp"
#include "ports/output/IEnvironment.hpp"

TEST(JsonProcessorTest, ProcessValidJson)
{
    auto converter = std::make_shared<JsonToEnvConverter>();
    JsonProcessor processor(converter);

    SimpleRequest req("POST", "/api", R"({"name": "admin", "age": 30})", "127.0.0.1", 80);
    req.setHeader("Content-Type", "application/json");
    SimpleResponse res;

    processor.handle(req, res);

    auto obj = req.getObject(IJsonProcessor::JSON_OBJECT_KEY);
    ASSERT_TRUE(obj.has_value());
    auto env = std::static_pointer_cast<IEnvironment>(*obj);
    EXPECT_EQ(env->get<std::string>("name"), "admin");
    EXPECT_EQ(env->get<int>("age"), 30);
}

TEST(JsonProcessorTest, ProcessInvalidJsonThrows)
{
    auto converter = std::make_shared<JsonToEnvConverter>();
    JsonProcessor processor(converter);

    SimpleRequest req("POST", "/api", "not json", "127.0.0.1", 80);
    req.setHeader("Content-Type", "application/json");
    SimpleResponse res;

    EXPECT_THROW(processor.handle(req, res), ConvertError);
}

TEST(JsonProcessorTest, NonJsonContentTypeThrows)
{
    auto converter = std::make_shared<JsonToEnvConverter>();
    JsonProcessor processor(converter);

    SimpleRequest req("POST", "/api", R"({"name": "admin"})", "127.0.0.1", 80);
    req.setHeader("Content-Type", "text/plain");
    SimpleResponse res;

    EXPECT_THROW(processor.handle(req, res), BadRequestError);
}

TEST(JsonProcessorTest, NameReturnsCorrectValue)
{
    auto converter = std::make_shared<JsonToEnvConverter>();
    JsonProcessor processor(converter);

    EXPECT_EQ(processor.name(), "JsonProcessor");
}

TEST(JsonProcessorTest, ProcessEmptyJsonObject)
{
    auto converter = std::make_shared<JsonToEnvConverter>();
    JsonProcessor processor(converter);

    SimpleRequest req("POST", "/api", "{}", "127.0.0.1", 80);
    req.setHeader("Content-Type", "application/json");
    SimpleResponse res;

    processor.handle(req, res);

    auto obj = req.getObject(IJsonProcessor::JSON_OBJECT_KEY);
    ASSERT_TRUE(obj.has_value());
}