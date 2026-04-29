#include "adapters/primary/BoostBeastApplication.hpp"
#include "version.hpp"
#include "adapters/primary/BeastRequestAdapter.hpp"
#include "adapters/primary/BeastResponseAdapter.hpp"
#include "adapters/secondary/Environment.hpp"
#include "adapters/primary/RouteMatcher.hpp"
#include "domain/HttpError.hpp"
#include "domain/error/MethodNotAllowedError.hpp"
#include "util/StringUtils.hpp"
#include "settings/ServerSettings.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <fstream>
#include <thread>

/**
 * @file BoostBeastApplication.cpp
 * @brief BoostBeastApplication implementation
 * @author Anton Tobolkin
 */

using json = nlohmann::json;

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

/**
 * @brief Construct BoostBeastApplication with logger
 * @param logger Logger instance (default: NullLogger)
 *
 * Default values:
 * - maxRequestBodySize: 16 MB
 * - readTimeout: 30000 ms
 * - writeTimeout: 30000 ms
 * - maxConnections: 1024 (prevents DoS)
 * - maxRequestsPerConnection: 100
 *
 * All values can be overridden via config.json or environment variables.
 */
BoostBeastApplication::BoostBeastApplication(std::shared_ptr<ILogger> logger)
    : maxRequestBodySize_(16 * 1024 * 1024),
      readTimeout_(30000), writeTimeout_(30000),
      maxConnections_(1024), maxRequestsPerConnection_(100),
      activeConnections_(0),
      logger_(std::move(logger))
{
    logger_->log(LogLevel::Info, "App", "BoostBeastApplication created");
}

/**
 * @brief Destructor - stops server if still running
 */
BoostBeastApplication::~BoostBeastApplication()
{
    if (state_.load(std::memory_order_acquire) == ServerState::Running)
    {
        shutdown();
    }
    logger_->log(LogLevel::Info, "App", "BoostBeastApplication destroyed");
}

/**
 * @brief Stop the server and wait for all sessions to finish
 *
 * Closes acceptor, stops io_context, and joins all worker threads.
 * Uses compare-exchange to ensure stop is called only once.
 */
void BoostBeastApplication::stop()
{
    ServerState expected = ServerState::Running;
    if (state_.compare_exchange_strong(expected, ServerState::Stopped))
    {
        logger_->log(LogLevel::Info, "App", "Stopping application...");

        if (acceptor_ && acceptor_->is_open())
        {
            acceptor_->close();
        }

        if (ioContext_)
        {
            ioContext_->stop();
        }

        logger_->log(LogLevel::Info, "App", "Waiting for sessions to finish...");
        std::vector<std::thread> threadsToJoin;
        {
            std::lock_guard<std::mutex> lock(threadsMutex_);
            threadsToJoin = std::move(threads_);
        }
        for (auto &t : threadsToJoin)
        {
            if (t.joinable())
            {
                t.join();
            }
        }
        logger_->log(LogLevel::Info, "App", "All sessions finished");
    }
}

/**
 * @brief Shutdown server with timeout
 * @param timeoutMs Maximum time to wait for graceful shutdown
 *
 * Calls stop() and waits up to timeoutMs for completion.
 */
void BoostBeastApplication::shutdown(std::chrono::milliseconds timeoutMs)
{
    const auto deadline = std::chrono::steady_clock::now() + timeoutMs;
    stop();

    while (state_.load(std::memory_order_acquire) != ServerState::Stopped &&
           std::chrono::steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    if (state_.load(std::memory_order_acquire) != ServerState::Stopped)
    {
        logger_->log(LogLevel::Error, "BoostBeastApplication", "Shutdown timeout");
    }
}

/**
 * @brief Register HTTP handler for method and path pattern
 * @param method HTTP method (GET, POST, etc.)
 * @param pattern URL pattern with wildcards (e.g., "/api/users/-star-")
 * @param handler Handler to execute for matching requests
 *
 * @throws std::logic_error if called after server start()
 *
 * Patterns:
 * - Exact match: "/api/users"
 * - Wildcard: "/api/-star-" matches "/api/anything"
 * - Named param: not yet supported
 */
void BoostBeastApplication::registerHandler(
    const std::string &method,
    const std::string &pattern,
    std::shared_ptr<IHttpHandler> handler)
{
    if (state_.load() != ServerState::NotStarted)
    {
        throw std::logic_error("Cannot register handler after server has started");
    }

    handlers_[pattern][method] = std::move(handler);

    logger_->log(LogLevel::Info, "App",
                 "Registered: " + method + " " + pattern);
}

/**
 * @brief Find handler matching method and path
 * @param method HTTP method
 * @param path Request path
 * @return HandlerMatch with handler and matched pattern, or std::nullopt
 *
 * Search order:
 * 1. Exact path match
 * 2. Wildcard pattern match (via RouteMatcher)
 */
std::optional<BoostBeastApplication::HandlerMatch> BoostBeastApplication::findHandler(
    const std::string &method,
    const std::string &path)
{
    auto exactIt = handlers_.find(path);
    if (exactIt != handlers_.end())
    {
        auto methodIt = exactIt->second.find(method);
        if (methodIt != exactIt->second.end())
        {
            return HandlerMatch{methodIt->second, path};
        }
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (pattern.find('*') == std::string::npos)
        {
            continue;
        }

        if (RouteMatcher::matches(pattern, path))
        {
            auto methodIt = methodHandlers.find(method);
            if (methodIt != methodHandlers.end())
            {
                return HandlerMatch{methodIt->second, pattern};
            }
        }
    }

    return std::nullopt;
}

/**
 * @brief Check if path has any registered handlers
 * @param path Request path
 * @return true if path has handlers (even if wrong method)
 *
 * Used to distinguish 404 (path not found) from 405 (method not allowed).
 */
bool BoostBeastApplication::pathExists(const std::string &path)
{
    if (handlers_.find(path) != handlers_.end())
    {
        return true;
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (pattern.find('*') != std::string::npos && RouteMatcher::matches(pattern, path))
        {
            return true;
        }
    }

    return false;
}

/**
 * @brief Handle HTTP request via registered handler
 * @param req HTTP request
 * @param res HTTP response
 *
 * Calls findHandler(), then executes handler.
 * Catches HttpError and std::exception, returns appropriate error responses.
 */
void BoostBeastApplication::handleRequest(IRequest &req, IResponse &res)
{
    std::string path = req.getPath();
    std::string method = req.getMethod();

    logger_->log(LogLevel::Info, "App",
                 method + " " + path + " from " + req.getIp());

    auto match = findHandler(method, path);

    if (match)
    {
        try
        {
            req.setPathPattern(match->pattern);
            match->handler->handle(req, res);
        }
        catch (const HttpError &e)
        {
            logger_->log(LogLevel::Error, "App",
                         "HttpError: " + std::to_string(e.statusCode()) + " - " + e.message());
            res.setResult(e.statusCode(), "application/json",
                          R"({"error": ")" + StringUtils::escapeJson(e.message()) + R"("})");
        }
        catch (const std::exception &e)
        {
            logger_->log(LogLevel::Error, "App",
                         std::string("Handler error: ") + e.what());
            res.setResult(500, "application/json", "{\"error\": \"Internal server error\"}");
        }
    }
    else
    {
        if (pathExists(path))
        {
            throw MethodNotAllowedError("Method " + method + " not allowed for " + path);
        }
        logger_->log(LogLevel::Warn, "App", "No handler found");
        res.setResult(404, "application/json", "{\"error\": \"Not found\"}");
    }
}

/**
 * @brief Handle Boost.Beast HTTP request
 * @param req Boost.Beast request object
 * @param res Boost.Beast response object
 * @param clientIp Client IP address
 * @param port Local port
 *
 * Wraps Beast types in adapters and delegates to handleRequest().
 */
void BoostBeastApplication::handleBeastRequest(
    const http::request<http::string_body> &req,
    http::response<http::string_body> &res,
    const std::string &clientIp,
    int port)
{
    BeastRequestAdapter requestAdapter(req, clientIp, port);
    BeastResponseAdapter responseAdapter(res);

    handleRequest(requestAdapter, responseAdapter);
}

/**
 * @brief Start HTTP server
 *
 * Loads configuration from ServerSettings (ENV + config.json),
 * binds to host:port, and starts accepting connections.
 * Each connection is handled in a separate thread.
 *
 * Runs until stop() is called or error occurs.
 */
void BoostBeastApplication::start()
{
    try
    {
        ServerSettings serverSettings(env_);
        std::string host = serverSettings.getHost();
        int port = serverSettings.getPort();
        maxRequestBodySize_ = serverSettings.getMaxRequestBodySize();
        readTimeout_ = serverSettings.getReadTimeout();
        writeTimeout_ = serverSettings.getWriteTimeout();
        maxConnections_ = serverSettings.getMaxConnections();
        maxRequestsPerConnection_ = serverSettings.getMaxRequestsPerConnection();

        logger_->log(LogLevel::Info, "App", "Starting HTTP server...");

        ioContext_ = std::make_unique<asio::io_context>();

        auto const address = asio::ip::make_address(host);
        tcp::endpoint endpoint{address, static_cast<unsigned short>(port)};

        acceptor_ = std::make_unique<tcp::acceptor>(*ioContext_, endpoint);

        logger_->log(LogLevel::Info, "Server",
                     "Listening on " + host + ":" + std::to_string(port));

        std::string maxConnMsg = (maxConnections_ > 0)
            ? "Max connections limit: " + std::to_string(maxConnections_)
            : "Max connections: unlimited";
        logger_->log(LogLevel::Info, "Server", maxConnMsg);

        logger_->log(LogLevel::Info, "Server", "Server is ready to accept connections!");

        state_.store(ServerState::Running);

        while (state_.load() == ServerState::Running)
        {
            tcp::socket socket{*ioContext_};
            acceptor_->accept(socket);

            int current = activeConnections_.load();

            if (maxConnections_ > 0 && current >= static_cast<int>(maxConnections_))
            {
                logger_->log(LogLevel::Warn, "Server",
                             "Connection limit reached (" + std::to_string(current) +
                             "/" + std::to_string(maxConnections_) + "). Sending 503.");

                http::response<http::string_body> res{http::status::service_unavailable, 11};
                res.set(http::field::server, "cpp-http-server/" CPP_HTTP_SERVER_VERSION);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Service unavailable. Connection limit reached."})";
                res.prepare_payload();

                beast::error_code ec;
                http::write(socket, res, ec);
                socket.shutdown(tcp::socket::shutdown_both, ec);
                continue;
            }

            activeConnections_++;
            logger_->log(LogLevel::Info, "Server",
                         "New connection accepted (" + std::to_string(activeConnections_.load()) +
                         "/" + (maxConnections_ > 0 ? std::to_string(maxConnections_) : "unlimited") + ")");

            std::thread t([this](tcp::socket socket)
                          { handleSession(std::move(socket)); }, std::move(socket));
            {
                std::lock_guard<std::mutex> lock(threadsMutex_);
                threads_.push_back(std::move(t));
            }
        }
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "Server",
                     std::string("Error: ") + e.what());
        state_.store(ServerState::Stopped);
    }
}

/**
 * @brief Handle single client session
 * @param socket TCP socket for client connection
 *
 * Reads HTTP requests in a loop (keep-alive support).
 * Respects maxRequestsPerConnection limit.
 * Decrements activeConnections_ on exit.
 */
void BoostBeastApplication::handleSession(tcp::socket socket)
{
    beast::tcp_stream stream(std::move(socket));

    std::string clientIp = "0.0.0.0";
    int localPort = 80;
    try
    {
        auto remoteEp = stream.socket().remote_endpoint();
        clientIp = remoteEp.address().to_string();
        localPort = stream.socket().local_endpoint().port();
        logger_->log(LogLevel::Info, "Session",
                     "Client connected from: " + clientIp);
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "Session",
                     std::string("Failed to get client IP: ") + e.what());
    }

    beast::flat_buffer buffer{maxRequestBodySize_};
    int requestCount = 0;

    try
    {
        while (static_cast<size_t>(requestCount) < maxRequestsPerConnection_)
        {
            stream.expires_after(readTimeout_);

            http::request<http::string_body> req;
            http::read(stream, buffer, req);

            requestCount++;
            logger_->log(LogLevel::Info, "Session",
                         "Request #" + std::to_string(requestCount) + " on keep-alive connection from " + clientIp);

            if (req.body().size() > maxRequestBodySize_)
            {
                logger_->log(LogLevel::Error, "Session",
                             "Request body too large: " + std::to_string(req.body().size()) +
                             " bytes (max: " + std::to_string(maxRequestBodySize_) + ")");
                http::response<http::string_body> res{http::status::payload_too_large, req.version()};
                res.set(http::field::server, "cpp-http-server/" CPP_HTTP_SERVER_VERSION);
                res.set(http::field::content_type, "application/json");
                res.body() = R"({"error": "Payload too large"})";
                res.prepare_payload();
                res.keep_alive(false);
                stream.expires_after(writeTimeout_);
                http::write(stream, res);
                return;
            }

            logger_->log(LogLevel::Info, "Session",
                         std::string("Received request: ") +
                         std::string(req.method_string()) + " " + std::string(req.target()));

            http::response<http::string_body> res{http::status::ok, req.version()};
            res.set(http::field::server, "cpp-http-server/" CPP_HTTP_SERVER_VERSION);
            res.keep_alive(req.keep_alive());

            handleBeastRequest(req, res, clientIp, localPort);

            stream.expires_after(writeTimeout_);
            http::write(stream, res);

            logger_->log(LogLevel::Info, "Session",
                         "Response sent with status: " + std::to_string(res.result_int()));

            if (!req.keep_alive())
            {
                break;
            }

            buffer.consume(buffer.size());
        }

        if (static_cast<size_t>(requestCount) >= maxRequestsPerConnection_)
        {
            logger_->log(LogLevel::Info, "Session",
                         "Max requests per connection reached (" + std::to_string(requestCount) +
                         "/" + std::to_string(maxRequestsPerConnection_) + "), closing");
        }
    }
    catch (const beast::system_error &se)
    {
        if (se.code() == beast::error::timeout)
        {
            logger_->log(LogLevel::Error, "Session",
                         std::string("Timeout: ") + se.what());
        }
        else if (se.code() != http::error::end_of_stream &&
                 se.code() != beast::errc::not_connected)
        {
            logger_->log(LogLevel::Error, "Session",
                         std::string("Error: ") + se.what());
        }
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "Session",
                     std::string("Unexpected error: ") + e.what());
    }

    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_send, ec);

    if (ec && ec != beast::errc::not_connected)
    {
        logger_->log(LogLevel::Error, "Session",
                     std::string("Shutdown error: ") + ec.message());
    }

    activeConnections_--;
}

/**
 * @brief Load configuration from command line and config.json
 * @param argc Argument count
 * @param argv Argument values
 *
 * Creates Environment instance and loads settings from:
 * 1. Command line arguments (--max-body-size=, --read-timeout=, etc.)
 * 2. Environment variables
 * 3. config.json file
 */
void BoostBeastApplication::loadEnvironment(int argc, char *argv[])
{
    logger_->log(LogLevel::Info, "App", "Loading environment...");

    env_ = std::make_shared<Environment>();

    for (int i = 0; i < argc; ++i)
    {
        std::string arg(argv[i]);
        if (arg.rfind("--max-body-size=", 0) == 0)
        {
            maxRequestBodySize_ = std::stoull(arg.substr(15));
        }
        else if (arg.rfind("--read-timeout=", 0) == 0)
        {
            readTimeout_ = std::chrono::milliseconds(std::stoi(arg.substr(14)));
        }
        else if (arg.rfind("--write-timeout=", 0) == 0)
        {
            writeTimeout_ = std::chrono::milliseconds(std::stoi(arg.substr(15)));
        }
        else if (arg.rfind("--max-connections=", 0) == 0)
        {
            maxConnections_ = std::stoull(arg.substr(17));
        }
    }

    if (const char *env = std::getenv("MAX_REQUEST_BODY_SIZE"))
    {
        maxRequestBodySize_ = std::stoull(env);
    }
    if (const char *env = std::getenv("READ_TIMEOUT_MS"))
    {
        readTimeout_ = std::chrono::milliseconds(std::stoi(env));
    }
    if (const char *env = std::getenv("WRITE_TIMEOUT_MS"))
    {
        writeTimeout_ = std::chrono::milliseconds(std::stoi(env));
    }
    if (const char *env = std::getenv("MAX_CONNECTIONS"))
    {
        maxConnections_ = std::stoull(env);
    }
    if (const char *env = std::getenv("MAX_REQUESTS_PER_CONNECTION"))
    {
        maxRequestsPerConnection_ = std::stoull(env);
    }

    try
    {
        std::ifstream configFile("config.json");

        if (!configFile.is_open())
        {
            logger_->log(LogLevel::Info, "App", "config.json not found, using defaults");
            return;
        }

        logger_->log(LogLevel::Info, "App", "Reading config.json...");

        json config = json::parse(configFile);

        loadJsonToEnvironment(config);

        logger_->log(LogLevel::Info, "App", "Configuration loaded from config.json");
    }
    catch (const json::parse_error &e)
    {
        logger_->log(LogLevel::Error, "App",
                     std::string("JSON parse error: ") + e.what());
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "App",
                     std::string("Error loading config: ") + e.what());
    }
}

/**
 * @brief Recursively load JSON to environment
 * @param j JSON object
 * @param prefix Key prefix for nested objects
 *
 * Converts JSON structure to flat key-value pairs in Environment.
 */
void BoostBeastApplication::loadJsonToEnvironment(const json &j, const std::string &prefix)
{
    for (auto it = j.begin(); it != j.end(); ++it)
    {
        std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();

        if (it->is_object())
        {
            loadJsonToEnvironment(*it, key);
        }
        else if (it->is_string())
        {
            std::string value = it->get<std::string>();
            logger_->log(LogLevel::Debug, "App", "Setting: " + key + " = " + value);
            env_->setProperty(key, value);
        }
        else if (it->is_number_integer())
        {
            env_->setProperty(key, it->get<int>());
        }
        else if (it->is_number_unsigned())
        {
            env_->setProperty(key, static_cast<int>(it->get<unsigned int>()));
        }
        else if (it->is_boolean())
        {
            env_->setProperty(key, it->get<bool>());
        }
        else if (it->is_number_float())
        {
            env_->setProperty(key, it->get<double>());
        }
        else if (it->is_array())
        {
            logger_->log(LogLevel::Debug, "App", "Skipping array: " + key);
        }
    }
}