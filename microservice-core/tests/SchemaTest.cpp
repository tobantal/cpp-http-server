#include <gtest/gtest.h>
#include "domain/schema/Schema.hpp"
#include <string>

class OrderSchema : public Schema
{
public:
    OrderSchema()
    {
        field<std::string>("symbol").required().minLength(1).maxLength(10);
        field<int>("amount").required().min(1);
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

TEST(SchemaTest, FieldsAreRegistered)
{
    OrderSchema schema;
    ASSERT_EQ(schema.fields().size(), 4u);
    EXPECT_EQ(schema.fields()[0].name, "symbol");
    EXPECT_EQ(schema.fields()[1].name, "amount");
    EXPECT_EQ(schema.fields()[2].name, "price");
    EXPECT_EQ(schema.fields()[3].name, "active");
}

TEST(SchemaTest, RequiredField)
{
    OrderSchema schema;
    EXPECT_TRUE(schema.fields()[0].required);
    EXPECT_TRUE(schema.fields()[1].required);
    EXPECT_FALSE(schema.fields()[2].required);
    EXPECT_FALSE(schema.fields()[3].required);
}

TEST(SchemaTest, StringType)
{
    OrderSchema schema;
    EXPECT_EQ(schema.fields()[0].type, typeid(std::string));
}

TEST(SchemaTest, IntType)
{
    OrderSchema schema;
    EXPECT_EQ(schema.fields()[1].type, typeid(int));
}

TEST(SchemaTest, DoubleType)
{
    OrderSchema schema;
    EXPECT_EQ(schema.fields()[2].type, typeid(double));
}

TEST(SchemaTest, BoolType)
{
    OrderSchema schema;
    EXPECT_EQ(schema.fields()[3].type, typeid(bool));
}

TEST(SchemaTest, MinConstraint)
{
    OrderSchema schema;
    EXPECT_TRUE(schema.fields()[1].hasMin);
    EXPECT_EQ(schema.fields()[1].minVal, 1.0);
}

TEST(SchemaTest, MaxConstraint)
{
    OrderSchema schema;
    EXPECT_TRUE(schema.fields()[2].hasMax);
    EXPECT_EQ(schema.fields()[2].maxVal, 1000000.0);
}

TEST(SchemaTest, MinLengthConstraint)
{
    LoginSchema schema;
    EXPECT_TRUE(schema.fields()[0].hasMinLength);
    EXPECT_EQ(schema.fields()[0].minLength, 3u);
}

TEST(SchemaTest, MaxLengthConstraint)
{
    LoginSchema schema;
    EXPECT_TRUE(schema.fields()[0].hasMaxLength);
    EXPECT_EQ(schema.fields()[0].maxLength, 50u);
    EXPECT_TRUE(schema.fields()[1].hasMinLength);
    EXPECT_EQ(schema.fields()[1].minLength, 8u);
}

TEST(SchemaTest, NoConstraintsWhenNotSet)
{
    OrderSchema schema;
    EXPECT_FALSE(schema.fields()[3].hasMin);
    EXPECT_FALSE(schema.fields()[3].hasMax);
    EXPECT_FALSE(schema.fields()[3].hasMinLength);
    EXPECT_FALSE(schema.fields()[3].hasMaxLength);
}