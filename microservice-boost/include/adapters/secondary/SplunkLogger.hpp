#pragma once

#include "ports/output/ILogger.hpp"
#include "ports/output/IHttpClient.hpp"
#include "ports/output/IShutdown.hpp"
#include "settings/ISplunkLogSettings.hpp"
#include <boost/asio.hpp>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <chrono>

/**
 * @file SplunkLogger.hpp
 * @brief Splunk logger implementation via HTTP Event Collector (HEC)
 * @author Anton Tobolkin
 */

/**
 * @class SplunkLogger
 * @brief Logger that sends log entries to Splunk via HTTP Event Collector
 *
 * Features:
 * - Asynchronous sending (does not block application)
 * - Buffering: flushes when buffer reaches configured size or timer expires
 * - Uses ISplunkLogSettings for configuration
 * - Fallback logger for failed requests
 * - Implements IShutdown for graceful flush on shutdown
 *
 * Usage:
 * @code
 *   auto settings = std::make_shared<SplunkLogSettings>("APP");
 *   auto logger = std::make_shared<SplunkLogger>(
 *       httpClient,
 *       settings,
 *       std::make_shared<ConsoleLogger>()  // fallback
 *   );
 *   logger->log(LogLevel::Info, "App", "User logged in");
 * @endcode
 */
class SplunkLogger : public ILogger, public IShutdown
{
public:
    /**
     * @brief Construct SplunkLogger
     * @param httpClient HTTP client for sending logs
     * @param settings Splunk logger settings
     * @param fallbackLogger Logger for failed requests (default: NullLogger)
     */
    SplunkLogger(std::shared_ptr<IHttpClient> httpClient,
                 std::shared_ptr<ISplunkLogSettings> settings,
                 std::shared_ptr<ILogger> fallbackLogger = nullptr);

    ~SplunkLogger() override;

    /**
     * @brief Log a message (non-blocking)
     * @param level Log severity level (Debug, Info, Warn, Error)
     * @param category Log category (e.g., "App", "Auth")
     * @param message Log message text
     */
    void log(LogLevel level,
             std::string_view category,
             std::string_view message) override;

    /**
     * @brief Flush buffer immediately (blocks until all sent)
     *
     * Sends all pending log entries to Splunk synchronously.
     * Use this when you need guaranteed delivery before proceeding.
     */
    void flush();

    /**
     * @brief Stop async operations and cleanup
     *
     * Stops the timer and joins the worker thread.
     * Does NOT flush pending logs - use shutdown() for graceful flush.
     */
    void stop();

    /**
     * @brief Graceful shutdown - flush all pending logs
     * @param timeoutMs Maximum time to wait for flush (default: 5000ms)
     *
     * Flushes all pending logs within the timeout period.
     * Logs that cannot be sent within timeout are written to fallback logger.
     */
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    /**
     * @brief Get component name for INameable interface
     * @return Component name "SplunkLogger"
     */
    std::string name() const override { return "SplunkLogger"; }

protected:
    /**
     * @brief Format log entry as Splunk HEC JSON
     * @param entryJson JSON from LogEntry::toJson()
     * @return Formatted JSON with event, index, sourcetype
     */
    std::string formatEntry(const std::string& entryJson) const;

    /**
     * @struct LogEntry
     * @brief Single log entry stored in buffer
     */
    struct LogEntry
    {
        LogLevel level;
        std::string category;
        std::string message;
        std::chrono::system_clock::time_point timestamp;

        /**
         * @brief Convert to JSON string
         * @return JSON: {"level":"INFO","category":"App","message":"text","timestamp":1234567890}
         */
        std::string toJson() const;
    };

    /**
     * @brief Schedule next timer-based flush
     */
    void scheduleFlush();

    /**
     * @brief Timer callback
     * @param ec Boost error code (operation_aborted means stopped)
     */
    void onTimer(const boost::system::error_code& ec);

    /**
     * @brief Internal flush implementation
     *
     * Moves entries from buffer to sendBatch().
     * Thread-safe via bufferMutex_.
     */
    void doFlush();

    /**
     * @brief Send batch of entries to Splunk via HTTP
     * @param entries Vector of LogEntry to send
     *
     * Formats each entry as Splunk HEC JSON and sends via HTTP POST.
     * On failure, logs error to fallbackLogger.
     */
    void sendBatch(const std::vector<LogEntry>& entries);

    std::shared_ptr<IHttpClient> httpClient_;
    std::shared_ptr<ISplunkLogSettings> settings_;
    std::shared_ptr<ILogger> fallbackLogger_;

    std::vector<LogEntry> buffer_;
    mutable std::mutex bufferMutex_;

    boost::asio::io_context ioContext_;
    boost::asio::steady_timer timer_;
    std::thread workerThread_;
    bool stopped_ = false;
};