#pragma once

#include <string>

/**
 * @file INameable.hpp
 * @brief Interface for named components
 * @author Anton Tobolkin
 */

/**
 * @struct INameable
 * @brief Interface for components that have a name (for logging, metrics, etc.)
 *
 * Follows Interface Segregation Principle (ISP): `name()` is a separate
 * concern from `handle()`, `shutdown()`, etc. Both IHttpHandler and
 * IShutdown inherit from INameable.
 */
struct INameable
{
    virtual ~INameable() = default;

    /**
     * @brief Get component name for logging and metrics
     * @return Component name
     */
    virtual std::string name() const = 0;
};