#pragma once

#include "ports/output/IEventPublisher.hpp"
#include "ports/output/IEventConsumer.hpp"
#include "ports/output/ILogger.hpp"
#include "ports/output/IShutdown.hpp"
#include "metrics/IMetricsCollector.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <functional>

/**
 * @file RabbitMQAdapter.hpp
 * @brief RabbitMQ adapter with reconnection and lifecycle management
 * @author Anton Tobolkin
 */

class IEnvironment;

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

/**
 * @typedef ConnectionLostCallback
 * @brief Callback invoked when the RabbitMQ connection is lost
 */
using ConnectionLostCallback = std::function<void()>;

/**
 * @class ReconnectBoostAsioHandler
 * @brief Custom LibBoostAsioHandler that detects connection loss
 *
 * Overrides TcpHandler::onError, onClosed, onLost to detect
 * connection failures and trigger reconnection.
 */
class ReconnectBoostAsioHandler : public AMQP::LibBoostAsioHandler
{
public:
    explicit ReconnectBoostAsioHandler(boost::asio::io_context &ioContext,
                                       ConnectionLostCallback onLost)
        : AMQP::LibBoostAsioHandler(ioContext), onLost_(std::move(onLost)) {}

private:
    void onError(AMQP::TcpConnection *connection, const char *message) override
    {
        AMQP::LibBoostAsioHandler::onError(connection, message);
        if (onLost_) onLost_();
    }

    void onClosed(AMQP::TcpConnection *connection) override
    {
        AMQP::LibBoostAsioHandler::onClosed(connection);
        if (onLost_) onLost_();
    }

    void onLost(AMQP::TcpConnection *connection) override
    {
        AMQP::LibBoostAsioHandler::onLost(connection);
        if (onLost_) onLost_();
    }

    ConnectionLostCallback onLost_;
};

/**
 * @class RabbitMQAdapter
 * @brief RabbitMQ adapter implementing IEventPublisher and IEventConsumer
 *
 * Features:
 * - Lifecycle state machine: Idle → Connecting → Connected → Reconnecting
 * - Automatic reconnection with exponential backoff
 * - Thread-safe publish and subscribe
 * - Metrics integration (amqp_published_total, amqp_received_total, amqp_errors_total)
 * - Graceful shutdown via IShutdown
 * - ILogger integration
 *
 * Lifecycle:
 * 1. Create adapter (state = Idle)
 * 2. Call subscribe() to register handlers
 * 3. Call start() to connect (state → Connecting → Connected)
 * 4. On connection loss: state → Reconnecting, auto-reconnects with backoff
 * 5. Call stop() or shutdown() to disconnect (state → Idle)
 */
class RabbitMQAdapter : public IEventPublisher,
                        public IEventConsumer,
                        public IShutdown
{
public:
    RabbitMQAdapter(std::shared_ptr<RabbitMQSettings> settings,
                    std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>(),
                    std::shared_ptr<IMetricsCollector> metrics = nullptr);

    ~RabbitMQAdapter() override;

    void publish(const std::string &routingKey, const std::string &message) override;
    void subscribe(const std::vector<std::string> &routingKeys, EventHandler handler) override;
    void start() override;
    void stop() override;

    std::string name() const override { return "RabbitMQAdapter"; }
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    RabbitMQConnectionState getState() const { return state_; }

private:
    void connect();
    void setupQueue();
    void applyPendingBindings();
    void startConsuming();
    void scheduleReconnect();
    void reconnect();
    void handleConnectionLost();
    void transitionState(RabbitMQConnectionState newState);
    std::string stateToString(RabbitMQConnectionState state) const;

    std::shared_ptr<RabbitMQSettings> settings_;
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IMetricsCollector> metrics_;

    std::string exchangeName_;
    std::string queueName_;

    std::atomic<RabbitMQConnectionState> state_{RabbitMQConnectionState::Idle};
    std::atomic<bool> running_{false};

    boost::asio::io_context ioContext_;
    std::unique_ptr<ReconnectBoostAsioHandler> handler_;
    std::unique_ptr<AMQP::TcpConnection> connection_;
    std::unique_ptr<AMQP::TcpChannel> channel_;

    std::thread workerThread_;
    std::unique_ptr<boost::asio::steady_timer> reconnectTimer_;

    std::mutex handlersMutex_;
    std::unordered_map<std::string, std::vector<EventHandler>> handlers_;
    std::vector<std::string> pendingBindings_;

    static constexpr int kBaseBackoffMs = 1000;
    static constexpr int kMaxBackoffMs = 30000;
    static constexpr double kBackoffMultiplier = 2.0;
    int currentBackoffMs_ = kBaseBackoffMs;
};