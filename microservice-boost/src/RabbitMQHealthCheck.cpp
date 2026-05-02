#include "adapters/primary/RabbitMQHealthCheck.hpp"

const std::map<RabbitMQConnectionState, std::string> RabbitMQHealthCheck::stateNames_ = {
    {RabbitMQConnectionState::Idle, "Idle"},
    {RabbitMQConnectionState::Connecting, "Connecting"},
    {RabbitMQConnectionState::Connected, "Connected"},
    {RabbitMQConnectionState::Reconnecting, "Reconnecting"},
};

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

    auto it = stateNames_.find(state);
    status.message = (it != stateNames_.end()) ? it->second : "Unknown";

    return status;
}
