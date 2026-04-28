#pragma once

#include "ports/output/ILogger.hpp"
#include "ports/output/IHttpClient.hpp"
#include "ports/output/IShutdown.hpp"
#include "settings/IHttpLogSettings.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>

/**
 * @file HttpLogger.hpp
 * @brief HTTP logger implementation - sends logs via HTTP
 * @author Anton Tobolkin
 */

/**
 * @class HttpLogger
 * @brief Logger that sends log entries via HTTP POST to a configurable endpoint
 *
 * Features:
 * - Asynchronous sending (does not block application)
 * - Buffering: flushes when buffer reaches configured size or timer expires
 * - Uses IHttpLogSettings for configuration (no direct ENV access)
 * - Fallback logger for failed requests
 * - Implements IShutdown for graceful flush on shutdown
 *
 * Usage:
 * @code
 *   auto settings = std::make_shared<HttpLogSettings>("APP");
 *   auto logger = std::make_shared<HttpLogger>(
 *       httpClient,
 *       settings,
 *       std::make_shared<ConsoleLogger>()  // fallback
 *   );
 *   logger->log(LogLevel::Info, "App", "started");
 * @endcode
 */
class HttpLogger : public ILogger, public IShutdown
{
public:
    /**
     * @brief Construct HttpLogger
     * @param httpClient HTTP client for sending logs
     * @param settings Configuration settings
     * @param fallbackLogger Logger for failed requests (default: NullLogger)
     */
    HttpLogger(std::shared_ptr<IHttpClient> httpClient,
               std::shared_ptr<IHttpLogSettings> settings,
               std::shared_ptr<ILogger> fallbackLogger = nullptr);

    ~HttpLogger() override;

    /**
     * @brief Log a message (non-blocking)
     * @param level Log severity level
     * @param category Log category
     * @param message Log message
     */
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;

    /**
     * @brief Flush buffer immediately (blocks until all sent)
     */
    void flush();

    /**
     * @brief Stop async operations and cleanup
     */
    void stop();

    /**
     * @brief Graceful shutdown - flush all pending logs
     * @param timeoutMs Maximum time to wait for flush (default: 5000ms)
     */
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    /**
     * @brief Get component name for INameable
     * @return Component name
     */
    std::string name() const override { return "HttpLogger"; }

protected:
    /**
     * @brief Format log entry as JSON for HTTP body
     * @param entry Log entry (level, category, message, timestamp)
     * @return JSON string
     */
    virtual std::string formatEntry(const std::string& entryJson) const;

    struct LogEntry
    {
        LogLevel level;
        std::string category;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::string toJson() const;
    };

    void scheduleFlush();
    void onTimer(const boost::system::error_code& ec);
    void doFlush();
    void sendBatch(const std::vector<LogEntry>& entries);

    std::shared_ptr<IHttpClient> httpClient_;
    std::shared_ptr<IHttpLogSettings> settings_;
    std::shared_ptr<ILogger> fallbackLogger_;

    std::vector<LogEntry> buffer_;
    mutable std::mutex bufferMutex_;

    boost::asio::io_context ioContext_;
    boost::asio::steady_timer timer_;
    std::thread workerThread_;
    bool stopped_ = false;
};