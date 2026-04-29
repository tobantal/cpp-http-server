#pragma once

#include <string>

/**
 * @file IEventPublisher.hpp
 * @brief Output port for publishing domain events
 * @author Anton Tobolkin
 */

struct DomainEvent;

/**
 * @interface IEventPublisher
 * @brief Output port implemented by message broker adapters (RabbitMQAdapter, InMemoryEventBus)
 *
 * Provides two publish overloads: raw (routingKey + message string)
 * and typed (DomainEvent → toJson). The typed overload is non-virtual
 * and delegates to the virtual raw overload, so subclasses only
 * implement publish(string, string).
 */
class IEventPublisher {
public:
    virtual ~IEventPublisher() = default;

    /**
     * @brief Publish a raw event to the message broker
     * @param routingKey Routing key for message dispatch (e.g. "order.created")
     * @param message JSON-serialized event body
     */
    virtual void publish(const std::string& routingKey, const std::string& message) = 0;

    /**
     * @brief Publish a typed domain event
     *
     * Non-virtual — calls publish(event.eventType, event.toJson()).
     * Subclasses need not override this method.
     *
     * @param event Domain event whose eventType becomes routingKey
     */
    void publish(const DomainEvent& event);
};