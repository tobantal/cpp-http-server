#include "settings/SplunkLogSettings.hpp"
#include <cstdlib>
#include <string>

SplunkLogSettings::SplunkLogSettings(const std::string& prefix)
    : prefix_(prefix)
{
    url_ = getEnvOrDefault((prefix + "_SPLUNK_URL").c_str(),
                           "http://localhost:8088/services/collector");
    token_ = getEnvOrDefault((prefix + "_SPLUNK_TOKEN").c_str(), "");
    index_ = getEnvOrDefault((prefix + "_SPLUNK_INDEX").c_str(), "main");
    sourcetype_ = getEnvOrDefault((prefix + "_SPLUNK_SOURCETYPE").c_str(), "_json");

    bufferSize_ = std::stoul(getEnvOrDefault(
        (prefix + "_SPLUNK_BUFFER_SIZE").c_str(), "100"));
    flushIntervalSec_ = std::stoi(getEnvOrDefault(
        (prefix + "_SPLUNK_FLUSH_INTERVAL_SEC").c_str(), "5"));
}

std::string SplunkLogSettings::getUrl() const {
    return url_;
}

std::string SplunkLogSettings::getToken() const {
    return token_;
}

std::string SplunkLogSettings::getIndex() const {
    return index_;
}

std::string SplunkLogSettings::getSourceType() const {
    return sourcetype_;
}

size_t SplunkLogSettings::getBufferSize() const {
    return bufferSize_;
}

std::chrono::seconds SplunkLogSettings::getFlushInterval() const {
    return std::chrono::seconds(flushIntervalSec_);
}

std::string SplunkLogSettings::getEnvOrDefault(const char* name, const std::string& defaultValue) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : defaultValue;
}