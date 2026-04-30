#include "adapters/secondary/InMemoryEventBus.hpp"

// =========================================================================
// IEventPublisher
// =========================================================================

void InMemoryEventBus::publish(const std::string& routingKey,
                               const std::string& message) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    publishedMessages_.push_back({routingKey, message});

    if (state_ != State::Running) return;

    auto it = handlers_.find(routingKey);
    if (it == handlers_.end()) return;

    for (auto& handler : it->second) {
        if (exceptionPolicy_ == ExceptionPolicy::Catch) {
            try {
                handler(routingKey, message);
            } catch (const std::exception& e) {
                errors_.emplace_back(routingKey, e.what());
            } catch (...) {
                errors_.emplace_back(routingKey, "unknown exception");
            }
        } else {
            handler(routingKey, message);
        }
    }
}

// =========================================================================
// IEventConsumer
// =========================================================================

void InMemoryEventBus::subscribe(const std::vector<std::string>& routingKeys,
                                 EventHandler handler) {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    for (const auto& key : routingKeys) {
        handlers_[key].push_back(handler);
    }
}

void InMemoryEventBus::start() {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    state_ = State::Running;
}

void InMemoryEventBus::stop() {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    state_ = State::Idle;
}

// =========================================================================
// Test helpers
// =========================================================================

void InMemoryEventBus::clear() {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    handlers_.clear();
    publishedMessages_.clear();
    errors_.clear();
    state_ = State::Idle;
}

size_t InMemoryEventBus::handlerCount(const std::string& routingKey) const {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    auto it = handlers_.find(routingKey);
    return it == handlers_.end() ? 0 : it->second.size();
}

size_t InMemoryEventBus::subscriptionCount() const {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    return handlers_.size();
}

bool InMemoryEventBus::isRunning() const {
    std::lock_guard<std::mutex> lock(handlersMutex_);
    return state_ == State::Running;
}

void InMemoryEventBus::setExceptionPolicy(ExceptionPolicy policy) {
    exceptionPolicy_ = policy;
}

const std::vector<InMemoryEventBus::PublishedMessage>& InMemoryEventBus::publishedMessages() const {
    return publishedMessages_;
}

size_t InMemoryEventBus::publishedCount() const {
    return publishedMessages_.size();
}

const std::vector<std::pair<std::string, std::string>>& InMemoryEventBus::errors() const {
    return errors_;
}