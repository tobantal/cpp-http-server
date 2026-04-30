#pragma once

#include "ports/output/IEventPublisher.hpp"
#include "ports/input/IEventConsumer.hpp"
#include <map>
#include <vector>
#include <string>
#include <mutex>

/**
 * @file InMemoryEventBus.hpp
 * @brief Synchronous in-process event bus for unit testing
 * @author Anton Tobolkin
 */

/**
 * @class InMemoryEventBus
 * @brief Test double implementing both IEventPublisher and IEventConsumer
 *
 * Publishes events synchronously in the calling thread, enabling
 * deterministic unit tests without a real message broker.
 *
 * Lifecycle:
 *   1. subscribe() — register handlers (may be called before start)
 *   2. start()     — enable message dispatch
 *   3. publish()   — deliver messages to subscribed handlers
 *   4. stop()      — disable dispatch (messages still recorded)
 *
 * Exception handling:
 *   Default: ExceptionPolicy::Catch — exceptions in handlers are caught
 *   and stored in errors(), remaining handlers continue.
 *   Alternative: ExceptionPolicy::Propagate — first exception propagates
 *   to the caller of publish().
 *
 * @example
 *   auto bus = std::make_shared<InMemoryEventBus>();
 *   bus->subscribe({"order.create"}, handler);
 *   bus->start();
 *   bus->publish("order.create", R"({"order_id":"ord-1"})");
 *   EXPECT_EQ(bus->publishedCount(), 1);
 */
class InMemoryEventBus : public IEventPublisher,
                         public IEventConsumer {
public:
    enum class ExceptionPolicy {
        Catch,       ///< Exceptions caught, stored in errors(), remaining handlers continue
        Propagate    ///< First exception propagates to publish() caller
    };

    /**
     * @brief Lifecycle state
     *
     * Idle    — not started or after stop()
     * Running — after start(), dispatch enabled
     */
    enum class State : uint8_t {
        Idle,
        Running
    };

    struct PublishedMessage {
        std::string routingKey;
        std::string message;
    };

    InMemoryEventBus() = default;
    ~InMemoryEventBus() override = default;

    InMemoryEventBus(const InMemoryEventBus&) = delete;
    InMemoryEventBus& operator=(const InMemoryEventBus&) = delete;
    InMemoryEventBus(InMemoryEventBus&&) = delete;
    InMemoryEventBus& operator=(InMemoryEventBus&&) = delete;

    // =========================================================================
    // IEventPublisher
    // =========================================================================

    /**
     * @brief Publish a message to all handlers subscribed to routingKey
     *
     * If bus is not running (start() not called or stop() called),
     * message is recorded in publishedMessages_ but handlers are NOT invoked.
     * This mirrors RabbitMQAdapter behavior: publish without connection is a no-op.
     */
    void publish(const std::string& routingKey,
                 const std::string& message) override;

    // =========================================================================
    // IEventConsumer
    // =========================================================================

    /**
     * @brief Subscribe a handler to one or more routing keys
     *
     * May be called before start(). Multiple handlers per routing key supported.
     */
    void subscribe(const std::vector<std::string>& routingKeys,
                   EventHandler handler) override;

    /** @brief Enable message dispatch to handlers */
    void start() override;

    /** @brief Disable message dispatch to handlers */
    void stop() override;

    // =========================================================================
    // Test helpers
    // =========================================================================

    /** @brief All published messages, in order of publish() calls */
    const std::vector<PublishedMessage>& publishedMessages() const;

    /** @brief Number of published messages */
    size_t publishedCount() const;

    /** @brief Reset all state: messages, errors, subscriptions */
    void clear();

    /** @brief Number of handlers subscribed to a specific routing key */
    size_t handlerCount(const std::string& routingKey) const;

    /** @brief Number of distinct routing keys with at least one handler */
    size_t subscriptionCount() const;

    /** @brief Whether start() was called and stop() was not */
    bool isRunning() const;

    /** @brief Set exception handling policy (default: Catch) */
    void setExceptionPolicy(ExceptionPolicy policy);

    /** @brief Errors caught in Catch mode: (routingKey, what) pairs */
    const std::vector<std::pair<std::string, std::string>>& errors() const;

private:
    mutable std::mutex handlersMutex_;
    std::map<std::string, std::vector<EventHandler>> handlers_;
    std::vector<PublishedMessage> publishedMessages_;
    std::vector<std::pair<std::string, std::string>> errors_;
    State state_ = State::Idle;
    ExceptionPolicy exceptionPolicy_ = ExceptionPolicy::Catch;
};