#pragma once

/**
 * @file ExceptionPolicy.hpp
 * @brief How to handle exceptions thrown by event handlers
 * @author Anton Tobolkin
 */

/**
 * @enum ExceptionPolicy
 * @brief How to handle exceptions thrown by event handlers
 */
enum class ExceptionPolicy
{
    Catch,
    Propagate
};