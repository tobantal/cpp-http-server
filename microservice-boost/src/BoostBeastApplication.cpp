#include "adapters/primary/BoostBeastApplication.hpp"
#include "adapters/primary/BeastRequestAdapter.hpp"
#include "adapters/primary/BeastResponseAdapter.hpp"
#include "application/ChainHandler.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <fstream>
#include <sstream>

BoostBeastApplication::~BoostBeastApplication()
{
    if (state_.load(std::memory_order_acquire) == ServerState::Running)
    {
        shutdown();
    }
}

void BoostBeastApplication::start()
{
    std::lock_guard<std::mutex> lock(threadsMutex_);
    if (state_.load(std::memory_order_acquire) == ServerState::Running)
    {
        logger_->log(LogLevel::Warn, "BoostBeastApplication", "Already running");
        return;
    }

    loadEnvironment(0, nullptr);

    uint16_t port = 8080;
    if (const char *envPort = std::getenv("PORT"))
    {
        port = static_cast<uint16_t>(std::stoul(envPort));
    }

    auto address = boost::asio::ip::make_address("0.0.0.0");
    boost::asio::ip::tcp::endpoint endpoint(address, port);
    ioContext_ = std::make_unique<boost::asio::io_context>();
    acceptor_ = std::make_unique<boost::asio::ip::tcp::acceptor>(*ioContext_);

    boost::system::error_code ec;
    acceptor_->open(endpoint.protocol(), ec);
    if (ec)
    {
        logger_->log(LogLevel::Error, "BoostBeastApplication", "Failed to open acceptor: " + ec.message());
        return;
    }

    acceptor_->set_option(boost::asio::ip::tcp::acceptor::reuse_address(true), ec);
    acceptor_->bind(endpoint, ec);
    if (ec)
    {
        logger_->log(LogLevel::Error, "BoostBeastApplication", "Failed to bind to port " + std::to_string(port) + ": " + ec.message());
        return;
    }

    acceptor_->listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        logger_->log(LogLevel::Error, "BoostBeastApplication", "Failed to start listening: " + ec.message());
        return;
    }

    state_.store(ServerState::Running, std::memory_order_release);
    logger_->log(LogLevel::Info, "BoostBeastApplication", "Started on port " + std::to_string(port));

    const size_t poolSize = std::thread::hardware_concurrency();
    threads_.reserve(poolSize);

    for (size_t i = 0; i < poolSize; ++i)
    {
        threads_.emplace_back([this] {
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work =
                boost::asio::make_work_guard(*ioContext_);
            ioContext_->run();
        });
    }

    doAccept();
}

void BoostBeastApplication::stop()
{
    std::lock_guard<std::mutex> lock(threadsMutex_);
    if (state_.load(std::memory_order_acquire) != ServerState::Running)
    {
        return;
    }

    state_.store(ServerState::Stopped, std::memory_order_release);
    logger_->log(LogLevel::Info, "BoostBeastApplication", "Stopping");

    if (acceptor_)
    {
        boost::system::error_code ec;
        acceptor_->close(ec);
    }

    if (ioContext_)
    {
        ioContext_->stop();
    }

    for (auto &thread : threads_)
    {
        if (thread.joinable())
        {
            thread.join();
        }
    }

    threads_.clear();
    state_.store(ServerState::Stopped, std::memory_order_release);
    logger_->log(LogLevel::Info, "BoostBeastApplication", "Stopped");
}

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

void BoostBeastApplication::loadEnvironment(int argc, char *argv[])
{
    if (argc > 0 && argv != nullptr)
    {
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

    loadJsonToEnvironment(nlohmann::json::object());
}

void BoostBeastApplication::registerHandler(
    const std::string &method,
    const std::string &pattern,
    std::shared_ptr<IHttpHandler> handler)
{
    handlers_[method][pattern] = std::move(handler);
}

std::optional<BoostBeastApplication::HandlerMatch> BoostBeastApplication::findHandler(
    const std::string &method,
    const std::string &path)
{
    auto methodIt = handlers_.find(method);
    if (methodIt == handlers_.end())
    {
        return std::nullopt;
    }

    std::optional<HandlerMatch> bestMatch;
    size_t bestScore = 0;

    for (const auto &[pattern, handler] : methodIt->second)
    {
        std::vector<std::string> patternParts = split(pattern, '/');
        std::vector<std::string> pathParts = split(path, '/');

        if (patternParts.size() != pathParts.size())
        {
            continue;
        }

        size_t score = 0;
        bool matches = true;

        for (size_t i = 0; i < patternParts.size(); ++i)
        {
            if (patternParts[i] == pathParts[i])
            {
                score += 10;
            }
            else if (patternParts[i] == "*")
            {
                score += 5;
            }
            else if (!patternParts[i].empty() && patternParts[i][0] == ':')
            {
                score += 8;
            }
            else
            {
                matches = false;
                break;
            }
        }

        if (matches && score > bestScore)
        {
            bestScore = score;
            bestMatch = HandlerMatch{handler, pattern};
        }
    }

    return bestMatch;
}

bool BoostBeastApplication::pathExists(const std::string &path)
{
    for (const auto &[method, methodHandlers] : handlers_)
    {
        for (const auto &[pattern, handler] : methodHandlers)
        {
            if (pattern == path)
            {
                return true;
            }
        }
    }
    return false;
}

void BoostBeastApplication::doAccept()
{
    acceptor_->async_accept(
        [this](const boost::system::error_code &ec, boost::asio::ip::tcp::socket socket) {
            if (ec)
            {
                if (ec != boost::asio::error::operation_aborted)
                {
                    logger_->log(LogLevel::Error, "BoostBeastApplication", "Accept error: " + ec.message());
                }
                return;
            }

            if (state_.load(std::memory_order_acquire) == ServerState::Running)
            {
                handleSession(std::move(socket));
                doAccept();
            }
        });
}

void BoostBeastApplication::handleSession(boost::asio::ip::tcp::socket socket)
{
    ++activeConnections_;

    try
    {
        boost::beast::flat_buffer buffer;
        boost::beast::http::request<boost::beast::http::string_body> req;

        boost::beast::http::read(socket, buffer, req);

        const auto clientIp = socket.remote_endpoint().address().to_string();
        const auto port = socket.local_endpoint().port();

        boost::beast::http::response<boost::beast::http::string_body> res;
        handleBeastRequest(req, res, clientIp, port);

        res.prepare_payload();
        boost::beast::http::write(socket, res);
    }
    catch (const boost::system::system_error &e)
    {
        if (e.code() != boost::beast::http::error::end_of_stream &&
            e.code() != boost::asio::error::operation_aborted)
        {
            logger_->log(LogLevel::Error, "BoostBeastApplication", "Session error: " + std::string(e.what()));
        }
    }

    --activeConnections_;
}

void BoostBeastApplication::handleBeastRequest(
    const boost::beast::http::request<boost::beast::http::string_body> &req,
    boost::beast::http::response<boost::beast::http::string_body> &res,
    const std::string &clientIp,
    int port)
{
    BeastRequestAdapter request(req, clientIp, port);
    BeastResponseAdapter response(res);

    if (req.body().size() > maxRequestBodySize_)
    {
        res.result(boost::beast::http::status::payload_too_large);
        res.body() = "Request body too large";
        return;
    }

    const auto methodStr = std::string(req.method_string());
    const auto path = std::string(req.target());

    auto match = findHandler(methodStr, path);
    if (!match)
    {
        res.result(boost::beast::http::status::not_found);
        res.body() = "Endpoint not found: " + path;
        return;
    }

    handleRequest(request, response);
}

void BoostBeastApplication::handleRequest(IRequest &req, IResponse &res)
{
    try
    {
        auto match = findHandler(req.getMethod(), req.getPath());
        if (match)
        {
            match->handler->handle(req, res);
        }
    }
    catch (const std::exception &e)
    {
        logger_->log(LogLevel::Error, "BoostBeastApplication", "Request handling error: " + std::string(e.what()));
    }
}

void BoostBeastApplication::loadJsonToEnvironment(const nlohmann::json &j, const std::string &prefix)
{
    if (!j.is_object())
    {
        return;
    }

    for (auto &[key, value] : j.items())
    {
        std::string envKey = prefix.empty() ? key : prefix + "_" + key;
        if (value.is_object())
        {
            loadJsonToEnvironment(value, envKey);
        }
        else if (value.is_string())
        {
            setenv(envKey.c_str(), value.get<std::string>().c_str(), 1);
        }
    }
}

std::vector<std::string> BoostBeastApplication::split(const std::string &s, char delimiter)
{
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, delimiter))
    {
        parts.push_back(part);
    }
    return parts;
}
