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
    Catch,
    Propagate
};