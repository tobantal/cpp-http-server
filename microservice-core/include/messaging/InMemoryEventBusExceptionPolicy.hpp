#pragma once

/**
 * @file InMemoryEventBusExceptionPolicy.hpp
 * @brief How to handle exceptions thrown by event handlers
 * @author Anton Tobolkin
 */

/**
 * @enum InMemoryEventBusExceptionPolicy
 * @brief How to handle exceptions thrown by event handlers
 */
enum class InMemoryEventBusExceptionPolicy
{
    Catch,      ///< Catch exceptions, store in errors list, continue dispatching to remaining handlers
    Propagate   ///< Let exceptions propagate to the caller, abort dispatching
};