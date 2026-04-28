#include "adapters/secondary/SplunkLogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <cstdlib>
#include <sstream>

/**
 * @file SplunkLogger.cpp
 * @brief SplunkLogger implementation
 * @author Anton Tobolkin
 */

using namespace std::chrono_literals;

SplunkLogger::SplunkLogger(std::shared_ptr<IHttpClient> httpClient,
                           std::shared_ptr<ILogger> fallbackLogger)
    : HttpLogger(std::move(httpClient), std::move(fallbackLogger))
{
}

std::string SplunkLogger::getSplunkUrl() const
{
    const char* url = std::getenv("SPLUNK_URL");
    return url ? url : "http://localhost:8088/services/collector";
}

std::string SplunkLogger::getSplunkToken() const
{
    const char* token = std::getenv("SPLUNK_TOKEN");
    return token ? token : "";
}

std::string SplunkLogger::getSplunkIndex() const
{
    const char* index = std::getenv("SPLUNK_INDEX");
    return index ? index : "main";
}

std::string SplunkLogger::getSplunkSourceType() const
{
    const char* sourcetype = std::getenv("SPLUNK_SOURCETYPE");
    return sourcetype ? sourcetype : "_json";
}

std::string SplunkLogger::getHttpUrl() const
{
    return getSplunkUrl();
}

std::string SplunkLogger::getHttpAuth() const
{
    std::string token = getSplunkToken();
    if (token.empty()) {
        return "";
    }
    return "Splunk " + token;
}

std::map<std::string, std::string> SplunkLogger::getHttpHeaders() const
{
    std::map<std::string, std::string> headers;
    const char* additionalHeaders = std::getenv("SPLUNK_HEADERS");
    if (additionalHeaders) {
        std::stringstream ss(additionalHeaders);
        std::string pair;
        while (std::getline(ss, pair, ';')) {
            auto colonPos = pair.find(':');
            if (colonPos != std::string::npos) {
                std::string key = pair.substr(0, colonPos);
                std::string value = pair.substr(colonPos + 1);
                headers[key] = value;
            }
        }
    }
    return headers;
}

std::string SplunkLogger::formatEntry(const std::string& entryJson) const
{
    std::ostringstream oss;
    oss << "{"
        << "\"event\":" << entryJson << ","
        << "\"index\":\"" << getSplunkIndex() << "\","
        << "\"sourcetype\":\"" << getSplunkSourceType() << "\""
        << "}";
    return oss.str();
}