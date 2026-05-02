#pragma once

#include <vector>
#include <string>
#include <typeindex>
#include <cstddef>
#include <utility>

/**
 * @file Schema.hpp
 * @brief Base class for request schema definitions
 * @author Anton Tobolkin
 */

struct FieldDef;

template<typename T>
class FieldBuilder;

/**
 * @class Schema
 * @brief Describes validation rules for request fields
 *
 * Subclass in application code to define per-endpoint schemas:
 *
 * class OrderSchema : public Schema {
 * public:
 *     OrderSchema() {
 *         field<std::string>("symbol").required().minLength(1);
 *         field<int>("amount").required().min(1);
 *     }
 * };
 */
class Schema
{
public:
    virtual ~Schema() = default;

    template<typename T>
    FieldBuilder<T> field(const std::string& name);

    const std::vector<FieldDef>& fields() const;

protected:
    std::vector<FieldDef> fields_;
};

struct FieldDef
{
    std::string name;
    std::type_index type = typeid(void);
    bool required = false;
    bool hasMin = false;
    bool hasMax = false;
    double minVal = 0;
    double maxVal = 0;
    bool hasMinLength = false;
    bool hasMaxLength = false;
    size_t minLength = 0;
    size_t maxLength = 0;
};

template<typename T>
class FieldBuilder
{
public:
    explicit FieldBuilder(const std::string& name, std::vector<FieldDef>& fields)
        : def_{name, typeid(T)}, fields_(fields)
    {
    }

    FieldBuilder& required()
    {
        def_.required = true;
        return *this;
    }

    FieldBuilder& optional()
    {
        def_.required = false;
        return *this;
    }

    FieldBuilder& min(double val)
    {
        def_.hasMin = true;
        def_.minVal = val;
        return *this;
    }

    FieldBuilder& max(double val)
    {
        def_.hasMax = true;
        def_.maxVal = val;
        return *this;
    }

    FieldBuilder& minLength(size_t val)
    {
        def_.hasMinLength = true;
        def_.minLength = val;
        return *this;
    }

    FieldBuilder& maxLength(size_t val)
    {
        def_.hasMaxLength = true;
        def_.maxLength = val;
        return *this;
    }

    ~FieldBuilder()
    {
        fields_.push_back(std::move(def_));
    }

private:
    FieldDef def_;
    std::vector<FieldDef>& fields_;
};

template<typename T>
FieldBuilder<T> Schema::field(const std::string& name)
{
    return FieldBuilder<T>(name, fields_);
}

inline const std::vector<FieldDef>& Schema::fields() const
{
    return fields_;
}