#include "adapters/primary/CircuitBreakingHttpClient.hpp"

CircuitBreakingHttpClient::CircuitBreakingHttpClient(
    std::shared_ptr<IHttpClient> inner,
    std::shared_ptr<ICircuitBreaker> circuitBreaker,
    std::shared_ptr<ILogger> logger)
    : inner_(std::move(inner))
    , circuitBreaker_(std::move(circuitBreaker))
    , logger_(std::move(logger))
{
}

HttpClientResult CircuitBreakingHttpClient::send(const IRequest& request, IResponse& response)
{
    if (!circuitBreaker_->allowsCall())
    {
        logger_->log(LogLevel::Warn, "CircuitBreakingHttpClient",
            "Circuit open, rejecting request");
        return HttpClientResult{HttpClientError::ConnectionRefused,
            "Circuit breaker is open"};
    }

    auto result = inner_->send(request, response);

    if (result.ok())
    {
        circuitBreaker_->recordSuccess();
    }
    else
    {
        circuitBreaker_->recordFailure(result.error);
    }

    return result;
}