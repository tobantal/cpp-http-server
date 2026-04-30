#pragma once

#include <amqpcpp.h>
#include <amqpcpp/libboostasio.h>
#include <boost/asio.hpp>
#include <functional>

/**
 * @file ReconnectBoostAsioHandler.hpp
 * @brief Custom LibBoostAsioHandler that detects connection loss
 * @author Anton Tobolkin
 */

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