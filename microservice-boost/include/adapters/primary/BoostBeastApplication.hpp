#pragma once

#include "application/BaseWebApplication.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "adapters/secondary/Environment.hpp"
#include "version.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>

class BoostBeastApplication : public BaseWebApplication
{
public:
    explicit BoostBeastApplication(
        std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());
    virtual ~BoostBeastApplication();

    static std::string getVersion()
    {
        return CPP_HTTP_SERVER_VERSION;
    }

    void start() override;
    void stop() override;
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;
    std::string name() const override { return "BoostBeastApplication"; }
    void loadEnvironment(int argc, char *argv[]) override;

private:
    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::vector<std::thread> threads_;
    std::mutex threadsMutex_;
    size_t maxRequestBodySize_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;
    size_t maxConnections_;
    size_t maxRequestsPerConnection_;
    std::atomic<int> activeConnections_{0};

    void handleSession(boost::asio::ip::tcp::socket socket);
    void handleBeastRequest(
        const boost::beast::http::request<boost::beast::http::string_body> &req,
        boost::beast::http::response<boost::beast::http::string_body> &res,
        const std::string &clientIp,
        int port);

    void loadJsonToEnvironment(const nlohmann::json &j, const std::string &prefix = "");
};