#pragma once

#include "ports/output/ILogger.hpp"
#include "ports/output/IHttpClient.hpp"
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
 * - Buffering: flushes when buffer reaches 100 entries or timer expires (5 sec)
 * - Configurable via ENV: HTTP_URL, HTTP_AUTH, HTTP_HEADERS
 * - Fallback logger for failed requests
 *
 * Usage:
 * @code
 * auto logger = std::make_shared<HttpLogger>(
 *     std::make_shared<ConsoleLogger>()  // fallback
 * );
 * logger->log(LogLevel::Info, "App", "started");
 * @endcode
 */
class HttpLogger : public ILogger
{
public:
    /**
     * @brief Construct HttpLogger
     * @param httpClient HTTP client for sending logs
     * @param fallbackLogger Logger for failed requests (default: NullLogger)
     */
    explicit HttpLogger(std::shared_ptr<IHttpClient> httpClient,
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

protected:
    /**
     * @brief Get HTTP URL from environment
     * @return URL or empty string if not set
     */
    virtual std::string getHttpUrl() const;

    /**
     * @brief Get HTTP auth token from environment
     * @return Auth token or empty string
     */
    virtual std::string getHttpAuth() const;

    /**
     * @brief Get additional HTTP headers from environment
     * @return Headers map
     */
    virtual std::map<std::string, std::string> getHttpHeaders() const;

    /**
     * @brief Format log entry as JSON for HTTP body
     * @param entry Log entry (level, category, message, timestamp)
     * @return JSON string
     */
    virtual std::string formatEntry(const std::string& entryJson) const;

    /**
     * @brief Get maximum buffer size
     * @return Max entries before forced flush
     */
    virtual size_t getMaxBufferSize() const { return 100; }

    /**
     * @brief Get flush interval in seconds
     * @return Interval seconds
     */
    virtual std::chrono::seconds getFlushInterval() const { return std::chrono::seconds(5); }

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
    std::shared_ptr<ILogger> fallbackLogger_;

    std::vector<LogEntry> buffer_;
    mutable std::mutex bufferMutex_;

    boost::asio::io_context ioContext_;
    boost::asio::steady_timer timer_;
    std::thread workerThread_;
    bool stopped_ = false;

    static constexpr size_t kDefaultBufferSize = 100;
};