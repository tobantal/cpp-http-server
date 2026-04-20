#pragma once

#include "ports/output/IHttpClient.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <memory>

/**
 * @file HttpClient.hpp
 * @brief HTTP client implementation using Boost.Beast
 * @author Anton Tobolkin
 */

/**
 * @class HttpClient
 * @brief HTTP client based on Boost.Beast/Asio
 */
class HttpClient : public IHttpClient
{
public:
    /**
     * @brief Construct HttpClient with optional logger
     * @param logger Logger instance (defaults to NullLogger)
     */
    explicit HttpClient(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());

    /**
     * @brief Send an HTTP request
     * @param request HTTP request
     * @param response HTTP response to populate
     * @return Result indicating success or error type
     */
    HttpClientResult send(const IRequest& request, IResponse& response) override;

private:
    static constexpr int kDefaultConnectTimeoutMs = 5000;
    static constexpr int kDefaultReadTimeoutMs = 30000;
    static constexpr int kDefaultWriteTimeoutMs = 30000;

    std::chrono::milliseconds connectTimeout_;
    std::chrono::milliseconds readTimeout_;
    std::chrono::milliseconds writeTimeout_;
    std::shared_ptr<ILogger> logger_;

    HttpClientResult connect(boost::beast::tcp_stream& stream,
                              const std::string& host,
                              const std::string& port);

    HttpClientResult sendRequest(boost::beast::tcp_stream& stream,
                                  const IRequest& request);

    HttpClientResult readResponse(boost::beast::tcp_stream& stream,
                                   IResponse& response);

    void loadTimeoutFromEnv(const char* envVar, std::chrono::milliseconds& target, int defaultMs);
};
