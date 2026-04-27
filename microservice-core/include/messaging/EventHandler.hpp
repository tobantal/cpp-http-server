#pragma once

#include <string>
#include <functional>

/**
 * @file EventHandler.hpp
 * @brief Event handler callback type for messaging subsystem
 * @author Anton Tobolkin
 */

/**
 * @brief Callback type for processing incoming events
 *
 * Accepts routingKey and message as strings, matching the
 * string-based protocol of RabbitMQ and other message brokers.
 * Used by IEventConsumer::subscribe() and InMemoryEventBus.
 */
using EventHandler = std::function<void(const std::string& routingKey, const std::string& message)>;