#include "adapters/primary/BoostBeastApplication.hpp"
#include "adapters/primary/BeastRequestAdapter.hpp"
#include "adapters/primary/BeastResponseAdapter.hpp"
#include "adapters/secondary/Environment.hpp"
#include "settings/ServerSettings.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <fstream>
#include <thread>

using json = nlohmann::json;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
using tcp = asio::ip::tcp;

BoostBeastApplication::BoostBeastApplication(std::shared_ptr<ILogger> logger)
    : BaseWebApplication(std::move(logger)),
      maxRequestBodySize_(1048576),
      readTimeout_(30000), writeTimeout_(30000), keepAliveTimeout_(5000),
      maxConnections_(0), activeConnections_(0)
{
}

BoostBeastApplication::~BoostBeastApplication()
{
    stop();
}

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

void BoostBeastApplication::shutdown(std::chrono::milliseconds /*timeoutMs*/)
{
    stop();
}

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
        keepAliveTimeout_ = serverSettings.getKeepAliveTimeout();
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
            if (requestCount == 0)
            {
                stream.expires_after(readTimeout_);
            }
            else
            {
                stream.expires_after(keepAliveTimeout_);
            }

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

            res.prepare_payload();
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
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    if (ec && ec != beast::errc::not_connected)
    {
        logger_->log(LogLevel::Error, "Session",
                     std::string("Shutdown error: ") + ec.message());
    }

    activeConnections_--;
}

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

void BoostBeastApplication::loadEnvironment(int argc, char *argv[])
{
    logger_->log(LogLevel::Info, "App", "Loading environment...");

    (void)argc;
    (void)argv;

    env_ = std::make_shared<Environment>();

    try
    {
        std::ifstream configFile("config.json");

        if (!configFile.is_open())
        {
            logger_->log(LogLevel::Info, "App", "config.json not found");
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
        throw;
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "App",
                     std::string("Error loading config: ") + e.what());
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