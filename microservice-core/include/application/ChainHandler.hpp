#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "ports/input/IHttpErrorHandler.hpp"
#include "handler/HttpErrorSender.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "domain/HttpError.hpp"
#include <memory>
#include <vector>

/**
 * @file ChainHandler.hpp
 * @brief Middleware chain handler
 * @author Anton Tobolkin
 */

/**
 * @class ChainHandler
 * @brief Middleware chain — executes handlers sequentially
 *
 * Executes each handler in order of addition. On HttpError,
 * delegates to IHttpErrorHandler. On std::exception — delegates with HttpError(500, ...).
 * Automatically extracts/generates X-Trace-ID and passes it to response.
 */
class ChainHandler : public IHttpHandler
{
public:
    /**
     * @brief Create chain with NullLogger and default HttpErrorSender
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    explicit ChainHandler(Handlers &&...handlers)
        : logger_(std::make_shared<NullLogger>()),
          errorHandler_(std::make_shared<HttpErrorSender>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Create chain with specified logger and default HttpErrorSender
     * @param logger Logger (if nullptr — NullLogger is used)
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    ChainHandler(std::shared_ptr<ILogger> logger, Handlers &&...handlers)
        : logger_(logger ? std::move(logger) : std::make_shared<NullLogger>()),
          errorHandler_(std::make_shared<HttpErrorSender>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Create chain with specified logger and error handler
     * @param logger Logger (if nullptr — NullLogger is used)
     * @param errorHandler Error handler (if nullptr — HttpErrorSender is used)
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    ChainHandler(std::shared_ptr<ILogger> logger,
                 std::shared_ptr<IHttpErrorHandler> errorHandler,
                 Handlers &&...handlers)
        : logger_(logger ? std::move(logger) : std::make_shared<NullLogger>()),
          errorHandler_(errorHandler ? std::move(errorHandler) : std::make_shared<HttpErrorSender>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Execute handler chain
     * @param req HTTP request
     * @param res HTTP response
     */
    void handle(IRequest &req, IResponse &res) override;

private:
    std::vector<std::shared_ptr<IHttpHandler>> handlers_;
    std::shared_ptr<ILogger> logger_;
    std::shared_ptr<IHttpErrorHandler> errorHandler_;
};