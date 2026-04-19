#pragma once

#include "IWebApplication.hpp"
#include "IHttpHandler.hpp"
#include "ILogger.hpp"
#include "NullLogger.hpp"
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
 * @brief HTTP-сервер на основе Boost.Beast/Asio
 * @version 2.1
 * @author Anton Tobolkin
 */
class BoostBeastApplication : public IWebApplication
{
public:
    explicit BoostBeastApplication(
        std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());
    virtual ~BoostBeastApplication();

    void start() override;
    void stop();
    void loadEnvironment(int argc, char *argv[]) override;

protected:
    /**
     * @brief Зарегистрировать обработчик (внутренний метод)
     */
    void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) override;

private:
    // =========================================================================
    // ROUTING
    // =========================================================================

    /**
     * @brief Хранилище обработчиков: pattern → method → handler
     */
    std::map<std::string, std::map<std::string, std::shared_ptr<IHttpHandler>>> handlers_;

    /**
     * @brief Результат поиска handler'а
     */
    struct HandlerMatch
    {
        std::shared_ptr<IHttpHandler> handler;
        std::string pattern;
    };

    /**
     * @brief Найти handler для указанного метода и пути
     */
    std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);
    bool pathExists(const std::string &path);

    // =========================================================================
    // HTTP SERVER
    // =========================================================================

    std::unique_ptr<boost::asio::io_context> ioContext_;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> acceptor_;
    std::atomic<ServerState> state_{ServerState::NotStarted};
    std::vector<std::thread> threads_;
    std::mutex threadsMutex_;
    size_t maxRequestBodySize_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;
    size_t maxConnections_;
    std::atomic<int> activeConnections_{0};
    std::shared_ptr<ILogger> logger_;

    void handleSession(boost::asio::ip::tcp::socket socket);
    void handleBeastRequest(
        const boost::beast::http::request<boost::beast::http::string_body> &req,
        boost::beast::http::response<boost::beast::http::string_body> &res,
        const std::string &clientIp,
        int port);
    void handleRequest(IRequest &req, IResponse &res);

    // =========================================================================
    // CONFIGURATION
    // =========================================================================

    void loadJsonToEnvironment(const nlohmann::json &j, const std::string &prefix = "");
};
