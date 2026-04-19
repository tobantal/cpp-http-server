#include "HttpClient.hpp"
#include "NullLogger.hpp"
#include <cstdlib>

using tcp = boost::asio::ip::tcp;
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;

HttpClient::HttpClient()
    : connectTimeout_(kDefaultConnectTimeoutMs),
      readTimeout_(kDefaultReadTimeoutMs),
      writeTimeout_(kDefaultWriteTimeoutMs),
      logger_(std::make_shared<NullLogger>())
{
    loadTimeoutFromEnv("HTTP_CLIENT_CONNECT_TIMEOUT_MS", connectTimeout_, kDefaultConnectTimeoutMs);
    loadTimeoutFromEnv("HTTP_CLIENT_READ_TIMEOUT_MS", readTimeout_, kDefaultReadTimeoutMs);
    loadTimeoutFromEnv("HTTP_CLIENT_WRITE_TIMEOUT_MS", writeTimeout_, kDefaultWriteTimeoutMs);
}

void HttpClient::setLogger(std::shared_ptr<ILogger> logger)
{
    logger_ = logger ? logger : std::make_shared<NullLogger>();
}

void HttpClient::loadTimeoutFromEnv(const char* envVar, std::chrono::milliseconds& target, int /*defaultMs*/)
{
    const char* env = std::getenv(envVar);
    if (env) {
        try {
            int ms = std::stoi(env);
            if (ms > 0) {
                target = std::chrono::milliseconds(ms);
            }
        } catch (...) {
        }
    }
}

HttpClientResult HttpClient::connect(beast::tcp_stream& stream,
                                      const std::string& host,
                                      const std::string& port)
{
    asio::io_context& ioc = static_cast<asio::io_context&>(stream.get_executor().context());
    tcp::resolver resolver(ioc);

    boost::system::error_code resolveEc;
    auto results = resolver.resolve(host, port, resolveEc);
    if (resolveEc)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     "DNS resolve failed: " + resolveEc.message());
        return {HttpClientError::DnsFailed, resolveEc.message()};
    }

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

    if (timeoutOccurred)
    {
        logger_->log(LogLevel::Error, "HttpClient", "Connect timeout");
        return {HttpClientError::ConnectTimeout, "connect timeout"};
    }

    if (connectEc)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     "Connect error: " + connectEc.message());
        if (connectEc == asio::error::connection_refused)
        {
            return {HttpClientError::ConnectionRefused, connectEc.message()};
        }
        return {HttpClientError::UnknownError, connectEc.message()};
    }

    return {HttpClientError::None, ""};
}

HttpClientResult HttpClient::sendRequest(beast::tcp_stream& stream,
                                          const IRequest& request)
{
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

    stream.expires_after(writeTimeout_);
    beast::error_code writeEc;
    http::write(stream, req, writeEc);

    if (writeEc)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     "Write error: " + writeEc.message());
        if (writeEc == beast::error::timeout)
        {
            return {HttpClientError::WriteTimeout, "write timeout"};
        }
        return {HttpClientError::UnknownError, writeEc.message()};
    }

    return {HttpClientError::None, ""};
}

HttpClientResult HttpClient::readResponse(beast::tcp_stream& stream,
                                            IResponse& response)
{
    beast::flat_buffer buffer;
    http::response<http::string_body> res;

    stream.expires_after(readTimeout_);
    beast::error_code readEc;
    http::read(stream, buffer, res, readEc);

    if (readEc)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     "Read error: " + readEc.message());
        if (readEc == beast::error::timeout)
        {
            return {HttpClientError::ReadTimeout, "read timeout"};
        }
        return {HttpClientError::UnknownError, readEc.message()};
    }

    logger_->log(LogLevel::Info, "HttpClient",
                 "Received status: " + std::to_string(res.result_int()));

    response.setStatus(res.result_int());
    response.setBody(res.body());

    for (const auto& field : res)
    {
        response.setHeader(std::string(field.name_string()), std::string(field.value()));
    }

    return {HttpClientError::None, ""};
}

HttpClientResult HttpClient::send(const IRequest& request, IResponse& response)
{
    try
    {
        std::string portStr = std::to_string(request.getPort());

        logger_->log(LogLevel::Info, "HttpClient",
                     std::string(request.getMethod()) + " " +
                     request.getIp() + ":" + portStr + request.getPath());

        asio::io_context ioc;
        beast::tcp_stream stream(ioc);

        auto connectResult = connect(stream, request.getIp(), portStr);
        if (!connectResult.ok())
        {
            return connectResult;
        }

        auto sendResult = sendRequest(stream, request);
        if (!sendResult.ok())
        {
            return sendResult;
        }

        auto readResult = readResponse(stream, response);
        if (!readResult.ok())
        {
            return readResult;
        }

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        return {HttpClientError::None, ""};
    }
    catch (const std::exception& e)
    {
        logger_->log(LogLevel::Error, "HttpClient",
                     std::string("Error: ") + e.what());
        return {HttpClientError::UnknownError, e.what()};
    }
}