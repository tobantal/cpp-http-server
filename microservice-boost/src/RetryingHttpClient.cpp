#include "adapters/primary/RetryingHttpClient.hpp"

RetryingHttpClient::RetryingHttpClient(
    std::shared_ptr<IHttpClient> inner,
    std::shared_ptr<IHttpRetryExecutor> executor)
    : inner_(std::move(inner))
    , executor_(std::move(executor))
{
}

HttpClientResult RetryingHttpClient::send(const IRequest& request, IResponse& response) {
    return executor_->execute([this, &request, &response]() {
        return inner_->send(request, response);
    });
}