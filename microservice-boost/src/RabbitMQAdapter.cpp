#include "adapters/secondary/RabbitMQAdapter.hpp"

RabbitMQAdapter::RabbitMQAdapter(std::shared_ptr<RabbitMQSettings> settings,
                                  std::shared_ptr<ILogger> logger,
                                  std::shared_ptr<IMetricsCollector> metrics)
    : settings_(std::move(settings)), logger_(std::move(logger)), metrics_(std::move(metrics))
{
    exchangeName_ = settings_->getExchange();
    logger_->log(LogLevel::Info, "RabbitMQAdapter",
                 "Created for " + settings_->getHost() + ":" +
                     std::to_string(settings_->getPort()) + " exchange=" + exchangeName_);
}

RabbitMQAdapter::~RabbitMQAdapter()
{
    stop();
}

void RabbitMQAdapter::publish(const std::string &routingKey, const std::string &message)
{
    if (!channel_ || state_ != RabbitMQConnectionState::Connected)
    {
        logger_->log(LogLevel::Warn, "RabbitMQAdapter",
                     "Cannot publish: not connected (state=" + stateToString(state_) + ")");
        if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "publish"}});
        return;
    }

    try
    {
        channel_->publish(exchangeName_, routingKey, message);
        logger_->log(LogLevel::Debug, "RabbitMQAdapter",
                     "Published " + routingKey + " (" + std::to_string(message.size()) + " bytes)");
        if (metrics_) metrics_->increment("amqp_published_total", {{"routing_key", routingKey}});
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "RabbitMQAdapter",
                     std::string("Publish error: ") + e.what());
        if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "publish"}});
    }
}

void RabbitMQAdapter::subscribe(const std::vector<std::string> &routingKeys, EventHandler handler)
{
    std::lock_guard<std::mutex> lock(handlersMutex_);

    for (const auto &key : routingKeys)
    {
        handlers_[key].push_back(handler);
        pendingBindings_.push_back(key);
        logger_->log(LogLevel::Info, "RabbitMQAdapter", "Registered handler for: " + key);
    }

    if (state_ == RabbitMQConnectionState::Connected && channel_)
    {
        applyPendingBindings();
    }
}

void RabbitMQAdapter::start()
{
    if (running_) return;

    running_ = true;
    currentBackoffMs_ = kBaseBackoffMs;
    transitionState(RabbitMQConnectionState::Connecting);

    workerThread_ = std::thread([this]() {
        try
        {
            connect();
            ioContext_.run();
        }
        catch (const std::exception &e)
        {
            logger_->log(LogLevel::Error, "RabbitMQAdapter",
                         std::string("Worker error: ") + e.what());
        }
    });

    logger_->log(LogLevel::Info, "RabbitMQAdapter", "Started");
}

void RabbitMQAdapter::stop()
{
    if (!running_) return;

    running_ = false;
    transitionState(RabbitMQConnectionState::Idle);
    ioContext_.stop();

    if (reconnectTimer_)
    {
        reconnectTimer_->cancel();
    }

    if (workerThread_.joinable())
    {
        workerThread_.join();
    }

    channel_.reset();
    connection_.reset();
    handler_.reset();

    logger_->log(LogLevel::Info, "RabbitMQAdapter", "Stopped");
}

void RabbitMQAdapter::shutdown(std::chrono::milliseconds timeoutMs)
{
    (void)timeoutMs;
    stop();
}

void RabbitMQAdapter::handleConnectionLost()
{
    if (!running_) return;

    if (state_ == RabbitMQConnectionState::Reconnecting) return;

    logger_->log(LogLevel::Warn, "RabbitMQAdapter", "Connection lost");
    if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "connection"}});
    scheduleReconnect();
}

void RabbitMQAdapter::connect()
{
    std::string connStr = settings_->getConnectionString();
    logger_->log(LogLevel::Info, "RabbitMQAdapter",
                 "Connecting to " + settings_->getHost() + ":" +
                     std::to_string(settings_->getPort()));

    handler_ = std::make_unique<ReconnectBoostAsioHandler>(ioContext_,
        [this]() { handleConnectionLost(); });
    connection_ = std::make_unique<AMQP::TcpConnection>(handler_.get(), AMQP::Address(connStr));
    channel_ = std::make_unique<AMQP::TcpChannel>(connection_.get());

    channel_->declareExchange(exchangeName_, AMQP::topic, AMQP::durable)
        .onSuccess([this]() {
            logger_->log(LogLevel::Info, "RabbitMQAdapter",
                         "Exchange declared: " + exchangeName_);
            setupQueue();
        })
        .onError([this](const char *msg) {
            logger_->log(LogLevel::Error, "RabbitMQAdapter",
                         std::string("Exchange error: ") + msg);
            if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "exchange"}});
            scheduleReconnect();
        });
}

void RabbitMQAdapter::setupQueue()
{
    int queueFlags = AMQP::exclusive;
    std::string queueToDeclare = settings_->getQueueName();

    if (queueToDeclare.empty())
    {
        channel_->declareQueue(queueFlags)
            .onSuccess([this](const std::string &name, uint32_t, uint32_t) {
                queueName_ = name;
                logger_->log(LogLevel::Info, "RabbitMQAdapter",
                             "Queue declared: " + queueName_);
                transitionState(RabbitMQConnectionState::Connected);
                currentBackoffMs_ = kBaseBackoffMs;
                {
                    std::lock_guard<std::mutex> lock(handlersMutex_);
                    applyPendingBindings();
                }
                startConsuming();
            })
            .onError([this](const char *msg) {
                logger_->log(LogLevel::Error, "RabbitMQAdapter",
                             std::string("Queue error: ") + msg);
                if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "queue"}});
                scheduleReconnect();
            });
    }
    else
    {
        queueName_ = queueToDeclare;
        channel_->declareQueue(queueName_, queueFlags)
            .onSuccess([this](const std::string &, uint32_t, uint32_t) {
                logger_->log(LogLevel::Info, "RabbitMQAdapter",
                             "Queue declared: " + queueName_);
                transitionState(RabbitMQConnectionState::Connected);
                currentBackoffMs_ = kBaseBackoffMs;
                {
                    std::lock_guard<std::mutex> lock(handlersMutex_);
                    applyPendingBindings();
                }
                startConsuming();
            })
            .onError([this](const char *msg) {
                logger_->log(LogLevel::Error, "RabbitMQAdapter",
                             std::string("Queue error: ") + msg);
                if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "queue"}});
                scheduleReconnect();
            });
    }
}

void RabbitMQAdapter::applyPendingBindings()
{
    if (pendingBindings_.empty() || !channel_) return;

    logger_->log(LogLevel::Info, "RabbitMQAdapter",
                 "Applying " + std::to_string(pendingBindings_.size()) + " bindings...");

    for (const auto &key : pendingBindings_)
    {
        channel_->bindQueue(exchangeName_, queueName_, key)
            .onSuccess([this, key]() {
                logger_->log(LogLevel::Debug, "RabbitMQAdapter", "Bound: " + key);
            })
            .onError([this, key](const char *msg) {
                logger_->log(LogLevel::Error, "RabbitMQAdapter",
                             "Bind error for " + key + ": " + msg);
                if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "bind"}});
            });
    }
    pendingBindings_.clear();
}

void RabbitMQAdapter::startConsuming()
{
    if (!channel_ || queueName_.empty()) return;

    logger_->log(LogLevel::Info, "RabbitMQAdapter",
                 "Starting consumer on queue: " + queueName_);

    channel_->consume(queueName_)
        .onReceived([this](const AMQP::Message &msg, uint64_t tag, bool) {
            std::string routingKey = msg.routingkey();
            std::string body(msg.body(), msg.bodySize());

            logger_->log(LogLevel::Debug, "RabbitMQAdapter",
                         "Received " + routingKey + " (" + std::to_string(body.size()) + " bytes)");
            if (metrics_) metrics_->increment("amqp_received_total", {{"routing_key", routingKey}});

            std::lock_guard<std::mutex> lock(handlersMutex_);
            auto it = handlers_.find(routingKey);
            if (it != handlers_.end())
            {
                for (const auto &handler : it->second)
                {
                    try
                    {
                        handler(routingKey, body);
                    }
                    catch (const std::exception &e)
                    {
                        logger_->log(LogLevel::Error, "RabbitMQAdapter",
                                     std::string("Handler error: ") + e.what());
                        if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "handler"}});
                    }
                }
            }

            if (channel_) channel_->ack(tag);
        })
        .onError([this](const char *msg) {
            logger_->log(LogLevel::Error, "RabbitMQAdapter",
                         std::string("Consume error: ") + msg);
            if (metrics_) metrics_->increment("amqp_errors_total", {{"type", "consume"}});
        });
}

void RabbitMQAdapter::scheduleReconnect()
{
    if (!running_) return;

    transitionState(RabbitMQConnectionState::Reconnecting);
    logger_->log(LogLevel::Info, "RabbitMQAdapter",
                 "Scheduling reconnect in " + std::to_string(currentBackoffMs_) + "ms");

    if (!reconnectTimer_)
    {
        reconnectTimer_ = std::make_unique<boost::asio::steady_timer>(ioContext_);
    }

    reconnectTimer_->expires_after(std::chrono::milliseconds(currentBackoffMs_));
    reconnectTimer_->async_wait([this](const boost::system::error_code &ec) {
        if (ec || !running_) return;
        reconnect();
    });

    currentBackoffMs_ = std::min(static_cast<int>(currentBackoffMs_ * kBackoffMultiplier), kMaxBackoffMs);
}

void RabbitMQAdapter::reconnect()
{
    if (!running_) return;

    logger_->log(LogLevel::Info, "RabbitMQAdapter", "Reconnecting...");
    transitionState(RabbitMQConnectionState::Connecting);

    channel_.reset();
    connection_.reset();
    handler_.reset();

    connect();
}

void RabbitMQAdapter::transitionState(RabbitMQConnectionState newState)
{
    RabbitMQConnectionState oldState = state_.exchange(newState);
    if (oldState != newState)
    {
        logger_->log(LogLevel::Info, "RabbitMQAdapter",
                     "State: " + stateToString(oldState) + " -> " + stateToString(newState));
    }
}

std::string RabbitMQAdapter::stateToString(RabbitMQConnectionState state) const
{
    switch (state)
    {
    case RabbitMQConnectionState::Idle: return "Idle";
    case RabbitMQConnectionState::Connecting: return "Connecting";
    case RabbitMQConnectionState::Connected: return "Connected";
    case RabbitMQConnectionState::Reconnecting: return "Reconnecting";
    default: return "Unknown";
    }
}