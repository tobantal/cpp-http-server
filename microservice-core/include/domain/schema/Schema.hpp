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

    /**
     * @brief Start defining a field with type T
     * @tparam T Expected field type (std::string, int, double, bool)
     * @param name Field name (JSON key)
     * @return FieldBuilder for chaining constraints
     */
    template<typename T>
    FieldBuilder<T> field(const std::string& name);

    /**
     * @brief Get all field definitions
     * @return Vector of FieldDef
     */
    const std::vector<FieldDef>& fields() const;

protected:
    std::vector<FieldDef> fields_;
};

/**
 * @struct FieldDef
 * @brief Describes validation rules for a single field
 */
struct FieldDef
{
    std::string name;              ///< Field name (JSON key)
    std::type_index type = typeid(void); ///< Expected C++ type (typeid(string/int/double/bool))
    bool required = false;         ///< Whether field must be present
    bool hasMin = false;           ///< Whether min constraint is set
    bool hasMax = false;           ///< Whether max constraint is set
    double minVal = 0;             ///< Minimum numeric value (int/double)
    double maxVal = 0;             ///< Maximum numeric value (int/double)
    bool hasMinLength = false;     ///< Whether minLength constraint is set
    bool hasMaxLength = false;     ///< Whether maxLength constraint is set
    size_t minLength = 0;         ///< Minimum string length
    size_t maxLength = 0;         ///< Maximum string length
};

/**
 * @class FieldBuilder
 * @brief Fluent API for defining field constraints
 *
 * Returned by Schema::field<T>(). Chain methods to set
 * required, min/max, minLength/maxLength. Field is added
 * to Schema when FieldBuilder is destroyed.
 */
template<typename T>
class FieldBuilder
{
public:
    /**
     * @brief Construct field builder
     * @param name Field name
     * @param fields Reference to Schema's field list
     */
    explicit FieldBuilder(const std::string& name, std::vector<FieldDef>& fields)
        : def_{name, typeid(T)}, fields_(fields)
    {
    }

    /// @brief Mark field as required
    FieldBuilder& required()
    {
        def_.required = true;
        return *this;
    }

    /// @brief Mark field as optional
    FieldBuilder& optional()
    {
        def_.required = false;
        return *this;
    }

    /// @brief Set minimum numeric value (int/double fields)
    FieldBuilder& min(double val)
    {
        def_.hasMin = true;
        def_.minVal = val;
        return *this;
    }

    /// @brief Set maximum numeric value (int/double fields)
    FieldBuilder& max(double val)
    {
        def_.hasMax = true;
        def_.maxVal = val;
        return *this;
    }

    /// @brief Set minimum string length
    FieldBuilder& minLength(size_t val)
    {
        def_.hasMinLength = true;
        def_.minLength = val;
        return *this;
    }

    /// @brief Set maximum string length
    FieldBuilder& maxLength(size_t val)
    {
        def_.hasMaxLength = true;
        def_.maxLength = val;
        return *this;
    }

    /// @brief Destructor: adds field definition to Schema
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