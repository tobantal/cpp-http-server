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

    /**
     * @brief Stop server and wait for all sessions to finish
     */
    void stop() override;

    /**
     * @brief Graceful shutdown with timeout
     * @param timeoutMs Maximum time to wait (default 5000ms)
     */
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    /**
     * @brief Get server instance name
     * @return "BoostBeastApplication"
     */
    std::string name() const override { return "BoostBeastApplication"; }

    /**
     * @brief Load configuration from args, ENV, and config.json
     * @param argc Argument count
     * @param argv Argument values
     */
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

    /**
     * @brief Find handler matching method and path
     * @param method HTTP method (GET, POST, etc.)
     * @param path Request path
     * @return HandlerMatch with handler and pattern, or std::nullopt
     */
    std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);

    /**
     * @brief Check if path has any registered handler
     * @param path Request path
     * @return true if path exists (even if method not allowed)
     */
    bool pathExists(const std::string &path);

    /**
     * @brief Split string by delimiter
     * @param s Input string
     * @param delimiter Character to split on
     * @return Vector of string segments
     */
    std::vector<std::string> split(const std::string &s, char delimiter);

    /**
     * @brief Handle single client session
     * @param socket TCP socket
     */
    void handleSession(boost::asio::ip::tcp::socket socket);

    /**
     * @brief Handle Boost.Beast HTTP request
     * @param req Boost.Beast request
     * @param res Boost.Beast response
     * @param clientIp Client IP address
     * @param port Local port
     */
    void handleBeastRequest(
        const boost::beast::http::request<boost::beast::http::string_body> &req,
        boost::beast::http::response<boost::beast::http::string_body> &res,
        const std::string &clientIp,
        int port);

    /**
     * @brief Handle request via registered handler
     * @param req HTTP request
     * @param res HTTP response
     */
    void handleRequest(IRequest &req, IResponse &res);

    /**
     * @brief Recursively load JSON to environment
     * @param j JSON object
     * @param prefix Key prefix for nested objects
     */
    void loadJsonToEnvironment(const nlohmann::json &j, const std::string &prefix = "");

    /**
     * @brief Accept new connection (async)
     */
    void doAccept();

    /** @brief Boost Asio io context for async operations */
    std::unique_ptr<boost::asio::io_context> ioContext_;

    /** @brief TCP acceptor for incoming connections */
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;

    /** @brief Current server state */
    std::atomic<ServerState> state_{ServerState::NotStarted};

    /** @brief Worker threads for handling connections */
    std::vector<std::thread> threads_;

    /** @brief Mutex for thread-safe access to threads_ */
    std::mutex threadsMutex_;

    /** @brief Maximum request body size in bytes (default 16MB) */
    size_t maxRequestBodySize_;

    /** @brief Read timeout in milliseconds (default 30000) */
    std::chrono::milliseconds readTimeout_;

    /** @brief Write timeout in milliseconds (default 30000) */
    std::chrono::milliseconds writeTimeout_;

    /** @brief Maximum concurrent connections (default 1024, 0=unlimited) */
    size_t maxConnections_;

    /** @brief Maximum requests per connection (default 100) */
    size_t maxRequestsPerConnection_;

    /** @brief Current number of active connections */
    std::atomic<int> activeConnections_{0};

    /** @brief Logger instance */
    std::shared_ptr<ILogger> logger_;
};
