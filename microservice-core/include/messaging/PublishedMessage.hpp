#pragma once

#include <string>

/**
 * @file PublishedMessage.hpp
 * @brief Record of a published message for test assertions
 * @author Anton Tobolkin
 */

/**
 * @struct PublishedMessage
 * @brief Record of a published message for test assertions
 */
struct PublishedMessage
{
    std::string routingKey;
    std::string message;
};