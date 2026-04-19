#pragma once

#include "IHttpClient.hpp"
#include "ILogger.hpp"
#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <memory>

class HttpClient : public IHttpClient
{
public:
    HttpClient();

    void setLogger(std::shared_ptr<ILogger> logger);

    bool send(const IRequest& request, IResponse& response) override;

private:
    static constexpr int kDefaultConnectTimeoutMs = 5000;
    std::chrono::milliseconds connectTimeout_;
    std::shared_ptr<ILogger> logger_;
};