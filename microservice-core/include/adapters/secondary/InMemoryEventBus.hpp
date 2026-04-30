#pragma once

#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "messaging/EventHandler.hpp"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <string>
#include <atomic>

/**
 * @file InMemoryEventBus.hpp
 * @brief In-memory event bus for testing and development
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

/**
 * @struct PublishedMessage
 * @brief Record of a published message for test assertions
 */
struct PublishedMessage
{
    std::string routingKey;
    std::string message;
};

/**
 * @class InMemoryEventBus
 * @brief Test double for IEventPublisher and IEventConsumer
 *
 * Implements both interfaces using an in-memory dispatch table.
 * Suitable for unit tests where a real message broker is not available.
 * Thread-safe: all operations are protected by a mutex.
 *
 * @example
 * @code
 * auto bus = std::make_shared<InMemoryEventBus>();
 * bus->subscribe({"order.created"}, [](auto key, auto msg) { ... });
 * bus->start();
 * bus->publish("order.created", R"({"id":42})");
 * @endcode
 */
class InMemoryEventBus : public IEventPublisher, public IEventConsumer
{
public:
    InMemoryEventBus();

    void publish(const std::string &routingKey, const std::string &message) override;
    void subscribe(const std::vector<std::string> &routingKeys, EventHandler handler) override;
    void start() override;
    void stop() override;

    size_t publishedCount() const;
    const std::vector<PublishedMessage> &publishedMessages() const;
    void clear();
    bool isRunning() const;
    size_t subscriptionCount() const;
    size_t handlerCount(const std::string &routingKey) const;
    void setExceptionPolicy(ExceptionPolicy policy);
    const std::vector<std::pair<std::string, std::string>> &errors() const;

    enum class State
    {
        Idle,
        Running
    };

private:
    mutable std::mutex handlersMutex_;
    std::atomic<ExceptionPolicy> exceptionPolicy_{ExceptionPolicy::Catch};
    State state_ = State::Idle;
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_;
    std::vector<PublishedMessage> publishedMessages_;
    std::vector<std::pair<std::string, std::string>> errors_;
};