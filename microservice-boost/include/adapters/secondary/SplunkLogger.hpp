#pragma once

#include "adapters/secondary/HttpLogger.hpp"
#include <memory>

/**
 * @file SplunkLogger.hpp
 * @brief Splunk logger implementation via HTTP Event Collector (HEC)
 * @author Anton Tobolkin
 */

/**
 * @class SplunkLogger
 * @brief Logger that sends log entries to Splunk via HTTP Event Collector
 *
 * Extends HttpLogger with Splunk-specific configuration and formatting:
 * - ENV: SPLUNK_URL, SPLUNK_TOKEN, SPLUNK_INDEX, SPLUNK_SOURCETYPE
 * - Automatic HEC authentication header
 * - Proper Splunk event format: {event, index, sourcetype, time}
 *
 * Usage:
 * @code
 * auto httpClient = std::make_shared<HttpClient>();
 * auto logger = std::make_shared<SplunkLogger>(
 *     httpClient,
 *     std::make_shared<ConsoleLogger>()  // fallback on failure
 * );
 * logger->log(LogLevel::Info, "App", "User logged in");
 * @endcode
 *
 * Docker setup for Splunk:
 * @code
 * docker run -d --name splunk -p 8000:8000 -p 8088:8088 \
 *   -e SPLUNK_START_ARGS=--accept-license \
 *   -e SPLUNK_PASSWORD=YourPassword123 \
 *   splunk/splunk:latest
 * @endcode
 * Then: Settings → Data inputs → HTTP Event Collector → Enable → Create token
 */
class SplunkLogger : public HttpLogger
{
public:
    /**
     * @brief Construct SplunkLogger
     * @param httpClient HTTP client for sending logs
     * @param fallbackLogger Logger for failed requests (default: NullLogger)
     */
    explicit SplunkLogger(std::shared_ptr<IHttpClient> httpClient,
                          std::shared_ptr<ILogger> fallbackLogger = nullptr);

protected:
    std::string getHttpUrl() const override;
    std::string getHttpAuth() const override;
    std::map<std::string, std::string> getHttpHeaders() const override;
    std::string formatEntry(const std::string& entryJson) const override;

    size_t getMaxBufferSize() const override { return 100; }
    std::chrono::seconds getFlushInterval() const override { return std::chrono::seconds(5); }

    std::string getSplunkUrl() const;
    std::string getSplunkToken() const;
    std::string getSplunkIndex() const;
    std::string getSplunkSourceType() const;
};