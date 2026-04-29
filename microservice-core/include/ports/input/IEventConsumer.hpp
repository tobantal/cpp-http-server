#pragma once

#include "messaging/EventHandler.hpp"
#include <vector>
#include <string>

/**
 * @file IEventConsumer.hpp
 * @brief Input port for consuming events from a message broker
 * @author Anton Tobolkin
 */

/**
 * @interface IEventConsumer
 * @brief Input port (driven by external message broker) for event consumption
 *
 * Uses a string-based interface compatible with RabbitMQ routing keys.
 * Lifecycle: subscribe() → start() → [deliver events] → stop().
 */
class IEventConsumer {
public:
    virtual ~IEventConsumer() = default;

    /**
     * @brief Subscribe a handler to one or more routing keys
     * @param routingKeys List of routing keys to subscribe to
     * @param handler Callback invoked when a matching message arrives
     */
    virtual void subscribe(const std::vector<std::string>& routingKeys, EventHandler handler) = 0;

    /**
     * @brief Start consuming messages from the broker
     *
     * Must be called after subscribe(). Dispatching starts only after start().
     */
    virtual void start() = 0;

    /**
     * @brief Stop consuming messages
     *
     * After stop(), handlers will not be invoked until start() is called again.
     * Published messages are still recorded but not dispatched.
     */
    virtual void stop() = 0;
};