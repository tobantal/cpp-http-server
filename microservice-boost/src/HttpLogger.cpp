#include "adapters/secondary/HttpLogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <cstdlib>
#include <sstream>
#include <thread>

/**
 * @file HttpLogger.cpp
 * @brief HttpLogger implementation
 * @author Anton Tobolkin
 */

using namespace std::chrono_literals;

HttpLogger::HttpLogger(std::shared_ptr<IHttpClient> httpClient,
                       std::shared_ptr<ILogger> fallbackLogger)
    : httpClient_(std::move(httpClient))
    , fallbackLogger_(fallbackLogger ? std::move(fallbackLogger) : std::make_shared<NullLogger>())
    , timer_(ioContext_)
{
    workerThread_ = std::thread([this]() { ioContext_.run(); });
    scheduleFlush();
}

HttpLogger::~HttpLogger()
{
    stop();
}

std::string HttpLogger::LogEntry::toJson() const
{
    std::ostringstream oss;
    oss << "{"
        << "\"level\":\"" << logLevelToString(level) << "\","
        << "\"category\":\"" << category << "\","
        << "\"message\":\"" << message << "\","
        << "\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(
            timestamp.time_since_epoch()).count()
        << "}";
    return oss.str();
}

std::string HttpLogger::getHttpUrl() const
{
    const char* url = std::getenv("HTTP_URL");
    return url ? url : "";
}

std::string HttpLogger::getHttpAuth() const
{
    const char* auth = std::getenv("HTTP_AUTH");
    return auth ? auth : "";
}

std::map<std::string, std::string> HttpLogger::getHttpHeaders() const
{
    std::map<std::string, std::string> headers;
    const char* headersStr = std::getenv("HTTP_HEADERS");
    if (headersStr) {
        std::stringstream ss(headersStr);
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

std::string HttpLogger::formatEntry(const std::string& entryJson) const
{
    return entryJson;
}

void HttpLogger::log(LogLevel level,
                     std::string_view category,
                     std::string_view message)
{
    LogEntry entry;
    entry.level = level;
    entry.category = std::string(category);
    entry.message = std::string(message);
    entry.timestamp = std::chrono::system_clock::now();

    bool shouldFlush = false;
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        buffer_.push_back(std::move(entry));
        if (buffer_.size() >= getMaxBufferSize()) {
            shouldFlush = true;
        }
    }
    if (shouldFlush) {
        ioContext_.post([this]() { doFlush(); });
    }
}

void HttpLogger::scheduleFlush()
{
    timer_.expires_after(getFlushInterval());
    timer_.async_wait([this](const boost::system::error_code& ec) {
        onTimer(ec);
    });
}

void HttpLogger::onTimer(const boost::system::error_code& ec)
{
    if (stopped_ || ec == boost::asio::error::operation_aborted) {
        return;
    }
    doFlush();
    scheduleFlush();
}

void HttpLogger::doFlush()
{
    std::vector<LogEntry> entries;
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        if (buffer_.empty()) {
            return;
        }
        entries = std::move(buffer_);
        buffer_.clear();
    }
    sendBatch(entries);
}

void HttpLogger::sendBatch(const std::vector<LogEntry>& entries)
{
    std::string url = getHttpUrl();
    if (url.empty()) {
        fallbackLogger_->log(LogLevel::Warn, "HttpLogger", "HTTP_URL not configured");
        return;
    }

    std::ostringstream body;
    for (size_t i = 0; i < entries.size(); ++i) {
        body << formatEntry(entries[i].toJson());
        if (i < entries.size() - 1) {
            body << "\n";
        }
    }

    class SimpleRequest : public IRequest
    {
    public:
        std::string getPath() const override { return path_; }
        std::vector<std::string> getPathSegments() const override { return {}; }
        std::string getPathPattern() const override { return {}; }
        void setPathPattern(const std::string&) override {}
        std::optional<std::string> getPathParam(size_t) const override { return std::nullopt; }
        std::map<std::string, std::string> getQueryParams() const override { return {}; }
        std::optional<std::string> getQueryParam(const std::string&) const override { return std::nullopt; }
        void setQueryParam(const std::string&, const std::string&) override {}
        std::map<std::string, std::string> getHeaders() const override { return headers_; }
        std::optional<std::string> getHeader(const std::string& name) const override {
            auto it = headers_.find(name);
            return it != headers_.end() ? std::optional<std::string>(it->second) : std::nullopt;
        }
        void setHeader(const std::string& name, const std::string& value) override {
            headers_[name] = value;
        }
        void setHeaders(const std::map<std::string, std::string>& h) override { headers_ = h; }
        std::string getBody() const override { return body_; }
        void setBody(const std::string& b) override { body_ = b; }
        std::string getMethod() const override { return "POST"; }
        std::string getIp() const override { return host_; }
        int getPort() const override { return port_; }
        std::optional<std::string> getBearerToken() const override { return std::nullopt; }
        bool isJson() const override { return true; }
        std::string getContentType() const override { return "application/json"; }
        void setAttribute(const std::string&, const std::string&) override {}
        std::optional<std::string> getAttribute(const std::string&) const override { return std::nullopt; }
        void setObject(const std::string&, std::shared_ptr<void>) override {}
        std::optional<std::shared_ptr<void>> getObject(const std::string&) const override { return std::nullopt; }
        std::string getTraceId() override { return {}; }
        void setTraceId(const std::string&) override {}

        std::string path_;
        std::string body_;
        std::string host_;
        int port_ = 80;
        std::map<std::string, std::string> headers_;
    };

    auto request = std::make_shared<SimpleRequest>();
    std::string urlPath = url.find("://") != std::string::npos ? url.substr(url.find("://") + 3) : url;
    size_t pathPos = urlPath.find('/');
    if (pathPos != std::string::npos) {
        request->host_ = urlPath.substr(0, pathPos);
        request->path_ = urlPath.substr(pathPos);
    } else {
        request->host_ = urlPath;
        request->path_ = "/";
    }
    size_t portPos = request->host_.find(':');
    if (portPos != std::string::npos) {
        request->port_ = std::stoi(request->host_.substr(portPos + 1));
        request->host_ = request->host_.substr(0, portPos);
    }
    request->body_ = body.str();
    request->headers_["Content-Type"] = "application/json";
    std::string auth = getHttpAuth();
    if (!auth.empty()) {
        request->headers_["Authorization"] = auth;
    }
    for (const auto& h : getHttpHeaders()) {
        request->headers_[h.first] = h.second;
    }

    class SimpleResponse : public IResponse
    {
    public:
        int status_ = 0;
        std::string body_;
        std::map<std::string, std::string> headers_;
        void setStatus(int code) override { status_ = code; }
        void setStatus(HttpStatus) override {}
        void setBody(const std::string& b) override { body_ = b; }
        void setHeader(const std::string& name, const std::string& value) override {
            headers_[name] = value;
        }
        void setCookie(const std::string&, const std::string&, const std::string&, bool, bool, int) override {}
        int getStatus() const override { return status_; }
        std::string getBody() const override { return body_; }
        std::map<std::string, std::string> getHeaders() const override { return headers_; }
        std::optional<std::string> getHeader(const std::string& name) const override {
            auto it = headers_.find(name);
            return it != headers_.end() ? std::optional<std::string>(it->second) : std::nullopt;
        }
        void setResult(int, const std::string&, const std::string&) override {}
        void setResult(HttpStatus, const std::string&, const std::string&) override {}
        void setTraceId(const std::string&) override {}
    };

    auto response = std::make_shared<SimpleResponse>();
    auto result = httpClient_->send(*request, *response);
    if (!result.ok()) {
        fallbackLogger_->log(LogLevel::Error, "HttpLogger",
            std::string("Failed to send logs: ") + result.errorMessage);
    }
}

void HttpLogger::flush()
{
    doFlush();
}

void HttpLogger::stop()
{
    stopped_ = true;
    timer_.cancel();
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
}

void HttpLogger::shutdown(std::chrono::milliseconds timeoutMs)
{
    stopped_ = true;
    timer_.cancel();

    auto start = std::chrono::steady_clock::now();
    auto remaining = [&]() {
        auto elapsed = std::chrono::steady_clock::now() - start;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
        return std::max(0, static_cast<int>(timeoutMs.count()) - static_cast<int>(ms));
    };

    std::vector<LogEntry> entries;
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        entries = std::move(buffer_);
        buffer_.clear();
    }

    if (!entries.empty()) {
        auto sendRemaining = remaining();
        if (sendRemaining > 0) {
            sendBatch(entries);
        } else {
            fallbackLogger_->log(LogLevel::Warn, "HttpLogger",
                "Shutdown timeout exceeded, dropping " + std::to_string(entries.size()) + " logs");
        }
    }

    if (workerThread_.joinable()) {
        auto waitMs = remaining();
        if (waitMs > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
        }
        if (workerThread_.joinable()) {
            workerThread_.detach();
        }
    }
}