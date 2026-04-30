#include "adapters/primary/RetryingHttpClient.hpp"

HttpClientResult RetryingHttpClient::send(const IRequest& request, IResponse& response) {
    return executor_->execute([this, &request, &response]() {
        return inner_->send(request, response);
    });
}