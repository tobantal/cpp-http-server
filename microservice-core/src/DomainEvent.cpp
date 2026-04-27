#include "domain/DomainEvent.hpp"
#include <chrono>

int64_t DomainEvent::currentTimestampMs() {
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch())
        .count();
}