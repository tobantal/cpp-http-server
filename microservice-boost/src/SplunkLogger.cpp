#include "adapters/secondary/SplunkLogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <sstream>

/**
 * @file SplunkLogger.cpp
 * @brief SplunkLogger implementation
 * @author Anton Tobolkin
 */

using namespace std::chrono_literals;

SplunkLogger::SplunkLogger(std::shared_ptr<IHttpClient> httpClient,
                           std::shared_ptr<ISplunkLogSettings> settings,
                           std::shared_ptr<ILogger> fallbackLogger)
    : HttpLogger(std::move(httpClient), settings, std::move(fallbackLogger))
    , splunkSettings_(std::dynamic_pointer_cast<SplunkLogSettings>(settings))
{
}

std::string SplunkLogger::formatEntry(const std::string& entryJson) const
{
    std::ostringstream oss;
    oss << "{"
        << "\"event\":" << entryJson << ","
        << "\"index\":\"" << splunkSettings_->getIndex() << "\","
        << "\"sourcetype\":\"" << splunkSettings_->getSourceType() << "\""
        << "}";
    return oss.str();
}