#pragma once

#include "adapters/secondary/HttpLogger.hpp"
#include "settings/SplunkLogSettings.hpp"
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
 * Uses SplunkLogSettings for configuration:
 * - <PREFIX>_SPLUNK_URL (default: http://localhost:8088/services/collector)
 * - <PREFIX>_SPLUNK_TOKEN
 * - <PREFIX>_SPLUNK_INDEX (default: main)
 * - <PREFIX>_SPLUNK_SOURCETYPE (default: _json)
 * - <PREFIX>_SPLUNK_BUFFER_SIZE, <PREFIX>_SPLUNK_FLUSH_INTERVAL_SEC
 *
 * Auth header is automatically set to "Splunk {token}".
 * Event format: {event: {...}, index: "...", sourcetype: "..."}
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
     * @param settings Splunk logger settings
     * @param fallbackLogger Logger for failed requests (default: NullLogger)
     */
    SplunkLogger(std::shared_ptr<IHttpClient> httpClient,
                 std::shared_ptr<SplunkLogSettings> settings,
                 std::shared_ptr<ILogger> fallbackLogger = nullptr);

protected:
    std::string formatEntry(const std::string& entryJson) const override;

    std::shared_ptr<SplunkLogSettings> splunkSettings_;
};