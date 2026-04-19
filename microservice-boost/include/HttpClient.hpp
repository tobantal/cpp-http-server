#pragma once

#include "IHttpClient.hpp"
#include "ILogger.hpp"
#include "NullLogger.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <memory>

class HttpClient : public IHttpClient
{
public:
    explicit HttpClient(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>());

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