#pragma once

#include <string>
#include <memory>

/**
 * @file IEventPublisher.hpp
 * @brief Interface for publishing events
 * @author Anton Tobolkin
 */

struct DomainEvent;

/**
 * @class IEventPublisher
 * @brief Interface for publishing events to a message bus
 *
 * Implementations may use RabbitMQ, Kafka, in-memory bus, etc.
 * Follows the Dependency Inversion Principle: domain code depends
 * on this abstraction, not on a specific messaging technology.
 */
class IEventPublisher
{
public:
    virtual ~IEventPublisher() = default;

    /**
     * @brief Publish an event to the message bus
     * @param routingKey Routing key for message dispatch (e.g. "order.created")
     * @param message Event payload (typically JSON)
     */
    virtual void publish(const std::string &routingKey, const std::string &message) = 0;

    /**
     * @brief Publish a domain event (convenience overload)
     * @param event Domain event to publish
     *
     * Default implementation calls publish(event.eventType, event.toJson()).
     * Can be overridden for custom serialization.
     */
    virtual void publish(const DomainEvent &event);
};