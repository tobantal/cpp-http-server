#include "ports/output/IEventPublisher.hpp"
#include "domain/DomainEvent.hpp"

void IEventPublisher::publish(const DomainEvent& event) {
    publish(event.eventType, event.toJson());
}