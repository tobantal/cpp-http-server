#pragma once

#include "ports/input/IHttpHandler.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include "domain/HttpError.hpp"
#include "util/StringUtils.hpp"
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
 * returns corresponding status. On std::exception — 500.
 * Automatically extracts/generates X-Trace-ID and passes it to response.
 */
class ChainHandler : public IHttpHandler
{
public:
    /**
     * @brief Create chain with NullLogger by default
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    explicit ChainHandler(Handlers &&...handlers)
        : logger_(std::make_shared<NullLogger>())
    {
        (handlers_.push_back(std::forward<Handlers>(handlers)), ...);
    }

    /**
     * @brief Create chain with specified logger
     * @param logger Logger (if nullptr — NullLogger is used)
     * @param handlers Handlers (shared_ptr<IHttpHandler>)
     */
    template <typename... Handlers>
    ChainHandler(std::shared_ptr<ILogger> logger, Handlers &&...handlers)
        : logger_(logger ? std::move(logger) : std::make_shared<NullLogger>())
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

    /**
     * @brief Send error response
     * @param res HTTP response
     * @param status HTTP status code
     * @param message Error message
     */
    void sendError(IResponse &res, int status, const std::string &message);
};
