#include "application/HttpRetryExecutor.hpp"
#include <thread>
#include <cmath>

HttpRetryExecutor::HttpRetryExecutor(
    std::shared_ptr<IHttpRetrySettings> settings,
    std::shared_ptr<ILogger> logger)
    : settings_(std::move(settings))
    , logger_(std::move(logger))
{
}

HttpClientResult HttpRetryExecutor::execute(const std::function<HttpClientResult()>& func)
{
    HttpClientResult lastResult;
    int maxAttempts = settings_->getMaxAttempts();

    for (int attempt = 1; attempt <= maxAttempts; ++attempt)
    {
        lastResult = func();

        if (lastResult.ok())
        {
            return lastResult;
        }

        if (!shouldRetryOnNetworkError(lastResult.error))
        {
            return lastResult;
        }

        if (attempt < maxAttempts)
        {
            logger_->log(LogLevel::Warn, "HttpRetry",
                std::to_string(attempt) + "/" + std::to_string(maxAttempts));
            auto delay = calculateDelay(attempt);
            std::this_thread::sleep_for(delay);
        }
    }

    return lastResult;
}

bool HttpRetryExecutor::shouldRetryOnHttpStatus(int statusCode) const
{
    return settings_->getRetryableStatuses().count(statusCode) > 0;
}

std::chrono::milliseconds HttpRetryExecutor::calculateDelay(int attempt) const
{
    auto baseDelay = settings_->getBaseDelay();
    auto multiplier = settings_->getMultiplier();
    auto maxDelay = settings_->getMaxDelay();

    double delayMs = baseDelay.count() * std::pow(multiplier, attempt - 1);
    auto calculatedDelay = static_cast<long long>(delayMs);

    if (calculatedDelay > maxDelay.count())
    {
        return maxDelay;
    }

    return std::chrono::milliseconds(calculatedDelay);
}

bool HttpRetryExecutor::shouldRetryOnNetworkError(HttpClientError error) const
{
    if (!settings_->isRetryOnNetworkErrorEnabled())
    {
        return false;
    }

    switch (error)
    {
        case HttpClientError::None:
            return false;
        case HttpClientError::DnsFailed:
        case HttpClientError::UnknownError:
            return false;
        default:
            return true;
    }
}