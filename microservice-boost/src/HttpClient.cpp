#include "HttpClient.hpp"
#include "NullLogger.hpp"
#include <cstdlib>

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

HttpClient::HttpClient()
    : connectTimeout_(kDefaultConnectTimeoutMs),
      logger_(std::make_shared<NullLogger>())
{
    const char* envTimeout = std::getenv("HTTP_CLIENT_CONNECT_TIMEOUT_MS");
    if (envTimeout) {
        try {
            int ms = std::stoi(envTimeout);
            if (ms > 0) {
                connectTimeout_ = std::chrono::milliseconds(ms);
            }
        } catch (...) {
        }
    }
}

void HttpClient::setLogger(std::shared_ptr<ILogger> logger)
{
    logger_ = logger ? logger : std::make_shared<NullLogger>();
}

bool HttpClient::send(const IRequest& request, IResponse& response)
{
    try
    {
        std::string portStr = std::to_string(request.getPort());

        logger_->log(LogLevel::Info, "HttpClient",
                     std::string(request.getMethod()) + " " +
                     request.getIp() + ":" + portStr + request.getPath());

        asio::io_context ioc;
        tcp::resolver resolver(ioc);
        auto results = resolver.resolve(request.getIp(), portStr);

        beast::tcp_stream stream(ioc);

        asio::steady_timer timer(ioc);
        timer.expires_after(connectTimeout_);

        bool timeoutOccurred = false;
        timer.async_wait([&](const beast::error_code& ec) {
            if (!ec) {
                timeoutOccurred = true;
                stream.close();
            }
        });

        beast::error_code connectEc;
        asio::async_connect(stream.socket(), results,
            [&](const beast::error_code& ec, const tcp::endpoint&) {
                timer.cancel();
                connectEc = ec;
            });

        ioc.run();

        if (timeoutOccurred || connectEc) {
            logger_->log(LogLevel::Error, "HttpClient",
                         "Connect error: " +
                         std::string(timeoutOccurred ? "timeout" : connectEc.message()));
            response.setStatus(500);
            response.setBody("Internal Server Error");
            return false;
        }

        http::request<http::string_body> req;
        req.method(http::string_to_verb(request.getMethod()));
        req.target(request.getPath());
        req.version(11);

        req.set(http::field::host, request.getIp());
        req.set(http::field::user_agent, "microservices/1.0");

        for (const auto& [key, value] : request.getHeaders())
        {
            req.set(key, value);
        }

        std::string body = request.getBody();
        if (!body.empty())
        {
            req.body() = body;
            req.set(http::field::content_length, std::to_string(body.length()));
        }

        req.prepare_payload();

        http::write(stream, req);

        beast::flat_buffer buffer;
        http::response<http::string_body> res;
        http::read(stream, buffer, res);

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        logger_->log(LogLevel::Info, "HttpClient",
                     "Received status: " + std::to_string(res.result_int()));

        response.setStatus(res.result_int());
        response.setBody(res.body());

        for (const auto& field : res)
        {
            response.setHeader(std::string(field.name_string()), std::string(field.value()));
        }

        return true;
    }
    catch (const std::exception& e)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     std::string("Error: ") + e.what());
        response.setStatus(500);
        response.setBody("Internal Server Error");
        return false;
    }
}