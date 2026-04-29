#pragma once

#include <string>
#include <memory>

/**
 * @file IConverter.hpp
 * @brief Base interface for type converters
 * @author Anton Tobolkin
 */

/**
 * @class IConverter
 * @brief Generic converter interface for transforming From type to To type
 * @tparam From Source type
 * @tparam To Target type
 */
template<typename From, typename To>
class IConverter
{
public:
    virtual ~IConverter() = default;

    /**
     * @brief Convert From type to To type
     * @param input Input value
     * @return Converted output
     * @throws ConvertError if conversion fails
     */
    virtual To convert(const From& input) const = 0;
};