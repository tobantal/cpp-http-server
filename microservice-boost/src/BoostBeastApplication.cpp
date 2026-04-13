#include "BoostBeastApplication.hpp"
#include "BeastRequestAdapter.hpp"
#include "BeastResponseAdapter.hpp"
#include "Environment.hpp"
#include "RouteMatcher.hpp"
#include "HttpError.hpp"
#include "settings/ServerSettings.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <fstream>
#include <thread>

using json = nlohmann::json;

namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// =============================================================================
// LIFECYCLE
// =============================================================================

BoostBeastApplication::BoostBeastApplication()
    : running_(false)
{
    std::cout << "[App] BoostBeastApplication created" << std::endl;
}

BoostBeastApplication::~BoostBeastApplication()
{
    stop();
    std::cout << "[App] BoostBeastApplication destroyed" << std::endl;
}

void BoostBeastApplication::stop()
{
    if (running_)
    {
        std::cout << "[App] Stopping application..." << std::endl;
        running_ = false;

        if (acceptor_ && acceptor_->is_open())
        {
            acceptor_->close();
        }

        if (ioContext_)
        {
            ioContext_->stop();
        }
    }
}

// =============================================================================
// ROUTING
// =============================================================================

void BoostBeastApplication::registerHandler(
    const std::string &method,
    const std::string &pattern,
    std::shared_ptr<IHttpHandler> handler)
{
    handlers_[pattern][method] = handler;

    std::cout << "[BoostBeastApplication] Registered: "
              << method << " " << pattern << std::endl;
}

std::optional<BoostBeastApplication::HandlerMatch> BoostBeastApplication::findHandler(
    const std::string &method,
    const std::string &path)
{
    // 1. Точное совпадение по паттерну
    auto exactIt = handlers_.find(path);
    if (exactIt != handlers_.end())
    {
        auto methodIt = exactIt->second.find(method);
        if (methodIt != exactIt->second.end())
        {
            return HandlerMatch{methodIt->second, path};
        }
    }

    // 2. Поиск по wildcard паттернам
    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        // Пропускаем exact matches (уже проверили)
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

// =============================================================================
// REQUEST HANDLING
// =============================================================================

void BoostBeastApplication::handleRequest(IRequest &req, IResponse &res)
{
    std::string path = req.getPath();
    std::string method = req.getMethod();

    std::cout << "[BoostBeastApplication] " << method << " " << path
              << " from " << req.getIp() << std::endl;

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
            std::cerr << "[BoostBeastApplication] HttpError: " << e.statusCode() << " - " << e.message() << std::endl;
            res.setResult(e.statusCode(), "application/json", R"({"error": ")" + e.message() + R"("})");
        }
        catch (const std::exception &e)
        {
            std::cerr << "[BoostBeastApplication] Handler error: " << e.what() << std::endl;
            res.setResult(500, "application/json", "{\"error\": \"Internal server error\"}");
        }
    }
    else
    {
        std::cout << "[BoostBeastApplication] No handler found" << std::endl;
        res.setResult(404, "application/json", "{\"error\": \"Not found\"}");
    }
}

void BoostBeastApplication::handleBeastRequest(
    const http::request<http::string_body> &req,
    http::response<http::string_body> &res,
    const std::string &clientIp)
{
    BeastRequestAdapter requestAdapter(req, clientIp);
    BeastResponseAdapter responseAdapter(res);

    handleRequest(requestAdapter, responseAdapter);
}

// =============================================================================
// HTTP SERVER
// =============================================================================

void BoostBeastApplication::start()
{
    try
    {
        ServerSettings serverSettings(env_);
        std::string host = serverSettings.getHost();
        int port = serverSettings.getPort();

        std::cout << "[App] Starting HTTP server..." << std::endl;

        ioContext_ = std::make_unique<asio::io_context>();

        auto const address = asio::ip::make_address(host);
        tcp::endpoint endpoint{address, static_cast<unsigned short>(port)};

        acceptor_ = std::make_unique<tcp::acceptor>(*ioContext_, endpoint);

        std::cout << "[Server] Listening on " << host << ":" << port << std::endl;
        std::cout << "[Server] Server is ready to accept connections!" << std::endl;

        running_ = true;

        while (running_)
        {
            tcp::socket socket{*ioContext_};
            acceptor_->accept(socket);

            std::cout << "[Server] New connection accepted" << std::endl;

            std::thread([this](tcp::socket socket)
                        { handleSession(std::move(socket)); }, std::move(socket))
                .detach();
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Server] Error: " << e.what() << std::endl;
        running_ = false;
    }
}

void BoostBeastApplication::handleSession(tcp::socket socket)
{
    try
    {
        std::string clientIp = "0.0.0.0";
        try
        {
            auto endpoint = socket.remote_endpoint();
            clientIp = endpoint.address().to_string();
            std::cout << "[Session] Client connected from: " << clientIp << std::endl;
        }
        catch (const std::exception &e)
        {
            std::cerr << "[Session] Failed to get client IP: " << e.what() << std::endl;
        }

        beast::flat_buffer buffer;

        http::request<http::string_body> req;
        http::read(socket, buffer, req);

        std::cout << "[Session] Received request: "
                  << req.method_string() << " " << req.target() << std::endl;

        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "BoostBeast");
        res.keep_alive(req.keep_alive());

        handleBeastRequest(req, res, clientIp);

        http::write(socket, res);

        std::cout << "[Session] Response sent with status: "
                  << res.result_int() << std::endl;

        beast::error_code ec;
        socket.shutdown(tcp::socket::shutdown_send, ec);

        if (ec && ec != beast::errc::not_connected)
        {
            std::cerr << "[Session] Shutdown error: " << ec.message() << std::endl;
        }
    }
    catch (const beast::system_error &se)
    {
        if (se.code() != http::error::end_of_stream &&
            se.code() != beast::errc::not_connected)
        {
            std::cerr << "[Session] Error: " << se.what() << std::endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "[Session] Unexpected error: " << e.what() << std::endl;
    }
}

// =============================================================================
// CONFIGURATION
// =============================================================================

void BoostBeastApplication::loadEnvironment(int argc, char *argv[])
{
    std::cout << "[BoostBeastApplication] Loading environment..." << std::endl;

    (void)argc;
    (void)argv;

    env_ = std::make_shared<Environment>();

    try
    {
        std::ifstream configFile("config.json");

        if (!configFile.is_open())
        {
            std::cout << "[BoostBeastApplication] config.json not found" << std::endl;
            return;
        }

        std::cout << "[BoostBeastApplication] Reading config.json..." << std::endl;

        json config = json::parse(configFile);

        loadJsonToEnvironment(config);

        std::cout << "[BoostBeastApplication] Configuration loaded from config.json" << std::endl;
    }
    catch (const json::parse_error &e)
    {
        std::cerr << "[BoostBeastApplication] JSON parse error: " << e.what() << std::endl;
        throw;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[BoostBeastApplication] Error loading config: " << e.what() << std::endl;
        throw;
    }
}

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
            std::cout << "[BoostBeastApplication] Setting: " << key << " = " << value << std::endl;
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
            std::cout << "[BoostBeastApplication] Skipping array: " << key << std::endl;
        }
    }
}
