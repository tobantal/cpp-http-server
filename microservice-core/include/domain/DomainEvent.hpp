#pragma once

#include <string>
#include <cstdint>
#include <memory>

/**
 * @file DomainEvent.hpp
 * @brief Base type for inter-service domain events
 * @author Anton Tobolkin
 */

/**
 * @struct DomainEvent
 * @brief Base class for all domain events exchanged between services
 *
 * Each event carries an eventType (used as routing key, e.g. "order.created")
 * and a timestamp (ms since epoch). Subclasses must implement toJson()
 * for serialization and clone() for polymorphic copying.
 *
 * DomainEvent is immutable after construction — no state transitions.
 */
struct DomainEvent {
    std::string eventType;   ///< Routing key, e.g. "order.created"
    int64_t timestamp = 0;   ///< ms since epoch (wire format)

    DomainEvent() = default;

    /**
     * @brief Create event with specified type and current timestamp
     */
    explicit DomainEvent(const std::string& type)
        : eventType(type), timestamp(currentTimestampMs()) {}

    virtual ~DomainEvent() = default;

    /**
     * @brief Serialize event to JSON string (snake_case keys)
     */
    virtual std::string toJson() const = 0;

    /**
     * @brief Polymorphic copy
     */
    virtual std::unique_ptr<DomainEvent> clone() const = 0;

protected:
    /**
     * @brief Current Unix timestamp in milliseconds
     */
    static int64_t currentTimestampMs();
};