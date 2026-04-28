#pragma once

#include "application/IWebApplication.hpp"
#include "ports/input/IHttpHandler.hpp"
#include "ports/output/IShutdown.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "application/ChainHandler.hpp"
#include "version.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/beast/http.hpp>
#include <nlohmann/json.hpp>
#include <memory>
#include <string>
#include <map>
#include <optional>
#include <atomic>
#include <vector>
#include <thread>
#include <cstdint>
#include <mutex>
#include <chrono>

/**
 * @enum ServerState
 * @brief Server state enum
 */
enum class ServerState : uint8_t
{
    NotStarted,
    Running,
    Stopped
};

class IRequest;
class IResponse;

/**
 * @class BoostBeastApplication
 * @brief HTTP server based on Boost.Beast/Asio
 * @version 2.1
 * @author Anton Tobolkin
 */
class BoostBeastApplication : public IWebApplication, public IShutdown
{
public:
    explicit BoostBeastApplication(
        std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());
    virtual ~BoostBeastApplication();

    /**
     * @brief Get library version string
     * @return Version string (e.g., "0.3.0")
     */
    static std::string getVersion()
    {
        return CPP_HTTP_SERVER_VERSION;
    }

    /**
     * @brief Register endpoint with auto-created ChainHandler
     * @param method HTTP method (GET, POST, ...)
     * @param pattern URL template with wildcards (e.g., "/api/v1/orders/")
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    void registerEndpoint(const std::string &method,
                          const std::string &pattern,
                          Handlers &&...handlers)
    {
        registerHandler(method, pattern,
                        std::make_shared<ChainHandler>(logger_, std::forward<Handlers>(handlers)...));
    }

    void start() override;
    void stop() override;
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;
    std::string name() const override { return "BoostBeastApplication"; }
    void loadEnvironment(int argc, char *argv[]) override;

protected:
    /**
     * @brief Register an HTTP handler (internal method)
     */
    void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) override;

private:
    std::map<std::string, std::map<std::string, std::shared_ptr<IHttpHandler>>> handlers_;

    struct HandlerMatch
    {
        std::shared_ptr<IHttpHandler> handler;
        std::string pattern;
    };

    std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);
    bool pathExists(const std::string &path);

    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::atomic<ServerState> state_{ServerState::NotStarted};
    std::vector<std::thread> threads_;
    std::mutex threadsMutex_;
    size_t maxRequestBodySize_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;
    std::chrono::milliseconds keepAliveTimeout_;
    size_t maxConnections_;
    size_t maxRequestsPerConnection_;
    std::atomic<int> activeConnections_{0};
    std::shared_ptr<ILogger> logger_;

    void handleSession(boost::asio::ip::tcp::socket socket);
    void handleBeastRequest(
        const boost::beast::http::request<boost::beast::http::string_body> &req,
        boost::beast::http::response<boost::beast::http::string_body> &res,
        const std::string &clientIp,
        int port);
    void handleRequest(IRequest &req, IResponse &res);

    void loadJsonToEnvironment(const nlohmann::json &j, const std::string &prefix = "");
};
