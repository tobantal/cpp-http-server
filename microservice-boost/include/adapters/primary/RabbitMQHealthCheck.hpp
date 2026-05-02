#pragma once

#include "ports/output/IHealthCheck.hpp"
#include "messaging/RabbitMQConnectionState.hpp"
#include <functional>
#include <map>
#include <string>

/**
 * @file RabbitMQHealthCheck.hpp
 * @brief Health check for RabbitMQ via connection state
 * @author Anton Tobolkin
 */

/**
 * @class RabbitMQHealthCheck
 * @brief IHealthCheck implementation for RabbitMQ message broker connectivity
 *
 * Checks connection state via a provided state accessor function.
 * Reports UP only when state is Connected.
 * Uses internal state-to-string map for status messages.
 */
class RabbitMQHealthCheck : public IHealthCheck
{
public:
    /**
     * @brief Construct RabbitMQHealthCheck with state accessor
     * @param stateAccessor Function that returns current connection state
     */
    explicit RabbitMQHealthCheck(std::function<RabbitMQConnectionState()> stateAccessor);

    /**
     * @brief Check RabbitMQ health
     * @return HealthStatus with name "rabbitmq" and current state
     */
    HealthStatus check() const override;

private:
    std::function<RabbitMQConnectionState()> stateAccessor_;

    static const std::map<RabbitMQConnectionState, std::string> stateNames_;
};
