#pragma once

#include <string>
#include <memory>

/**
 * @file RabbitMQSettings.hpp
 * @brief RabbitMQ configuration loaded from environment variables
 * @author Anton Tobolkin
 */

class IEnvironment;

/**
 * @class RabbitMQSettings
 * @brief RabbitMQ configuration loaded from environment variables
 *
 * Reads from ENV (with fallback defaults):
 * - RABBITMQ_HOST (default: "localhost")
 * - RABBITMQ_PORT (default: 5672)
 * - RABBITMQ_USER (default: "guest")
 * - RABBITMQ_PASSWORD (default: "guest")
 * - RABBITMQ_EXCHANGE (default: "events")
 * - RABBITMQ_QUEUE_NAME (default: "" — server-generated exclusive queue)
 */
class RabbitMQSettings
{
public:
    explicit RabbitMQSettings(std::shared_ptr<IEnvironment> env);

    std::string getHost() const { return host_; }
    int getPort() const { return port_; }
    std::string getUser() const { return user_; }
    std::string getPassword() const { return password_; }
    std::string getExchange() const { return exchange_; }
    std::string getQueueName() const { return queueName_; }
    std::string getConnectionString() const;

private:
    static std::string getEnvOrDefault(const char *name, const std::string &defaultValue);
    static int getEnvOrDefaultInt(const char *name, int defaultValue);

    std::string host_;
    int port_;
    std::string user_;
    std::string password_;
    std::string exchange_;
    std::string queueName_;

    static constexpr auto kDefaultHost = "localhost";
    static constexpr int kDefaultPort = 5672;
    static constexpr auto kDefaultUser = "guest";
    static constexpr auto kDefaultPassword = "guest";
    static constexpr auto kDefaultExchange = "events";
};