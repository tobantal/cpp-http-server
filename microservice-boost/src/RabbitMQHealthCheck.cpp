#include "adapters/primary/RabbitMQHealthCheck.hpp"

RabbitMQHealthCheck::RabbitMQHealthCheck(std::function<RabbitMQConnectionState()> stateAccessor)
    : stateAccessor_(std::move(stateAccessor))
{
}

HealthStatus RabbitMQHealthCheck::check() const
{
    HealthStatus status;
    status.name = "rabbitmq";

    auto state = stateAccessor_();
    status.healthy = (state == RabbitMQConnectionState::Connected);

    switch (state)
    {
    case RabbitMQConnectionState::Connected:
        status.message = "Connected";
        break;
    case RabbitMQConnectionState::Connecting:
        status.message = "Connecting";
        break;
    case RabbitMQConnectionState::Reconnecting:
        status.message = "Reconnecting";
        break;
    case RabbitMQConnectionState::Idle:
    default:
        status.message = "Idle";
        break;
    }

    return status;
}
