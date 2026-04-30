#pragma once

#include <cstdint>

/**
 * @file RabbitMQConnectionState.hpp
 * @brief RabbitMQ connection lifecycle states
 * @author Anton Tobolkin
 */

/**
 * @enum RabbitMQConnectionState
 * @brief Lifecycle states for the RabbitMQ connection
 */
enum class RabbitMQConnectionState : uint8_t
{
    Idle,
    Connecting,
    Connected,
    Reconnecting
};