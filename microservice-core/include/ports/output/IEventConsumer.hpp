#pragma once

#include "messaging/EventHandler.hpp"
#include <vector>
#include <string>

/**
 * @file IEventConsumer.hpp
 * @brief Interface for consuming events
 * @author Anton Tobolkin
 */

/**
 * @class IEventConsumer
 * @brief Interface for subscribing to and consuming events
 *
 * Lifecycle:
 * 1. Create consumer
 * 2. Call subscribe() to register handlers
 * 3. Call start() to begin consuming
 * 4. Call stop() to shut down
 *
 * @example
 * @code
 * consumer->subscribe({"order.created", "order.rejected"},
 *     [](const std::string& routingKey, const std::string& message) {
 *         // handle event
 *     });
 * consumer->start();
 * @endcode
 */
class IEventConsumer
{
public:
    virtual ~IEventConsumer() = default;

    /**
     * @brief Subscribe to events with specific routing keys
     * @param routingKeys List of routing keys to subscribe to
     * @param handler Callback invoked when a matching event is received
     */
    virtual void subscribe(const std::vector<std::string> &routingKeys, EventHandler handler) = 0;

    /**
     * @brief Start consuming events
     *
     * Must be called after all subscribe() calls.
     * Implementations may spawn a background thread.
     */
    virtual void start() = 0;

    /**
     * @brief Stop consuming events and release resources
     */
    virtual void stop() = 0;
};