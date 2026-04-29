#pragma once

#include <functional>

/**
 * @file IExecutorPolicy.hpp
 * @brief Template interface for retry execution
 * @author Anton Tobolkin
 */

/**
 * @class IExecutorPolicy
 * @brief Template interface for executing operations with retry
 * @tparam Result Type returned by the operation
 *
 * Service-specific executor that handles retry logic.
 * Each service (HTTP, DB, etc.) implements its own executor
 * with knowledge of service-specific error types and retry conditions.
 *
 * @par Usage
 * @code
 *   auto httpExecutor = std::make_shared<HttpRetryExecutor>(settings, policy);
 *   auto result = httpExecutor->execute([&]() { return httpClient->send(req, res); });
 * @endcode
 */
template<typename Result>
class IExecutorPolicy
{
public:
    virtual ~IExecutorPolicy() = default;

    /**
     * @brief Execute operation with retry logic
     * @param func Function to execute
     * @return Result of the operation (last attempt if all failed)
     */
    virtual Result execute(const std::function<Result()>& func) = 0;
};