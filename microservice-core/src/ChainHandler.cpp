#include "application/ChainHandler.hpp"
#include <chrono>

/**
 * @file ChainHandler.cpp
 * @brief ChainHandler implementation
 * @author Anton Tobolkin
 */

void ChainHandler::handle(IRequest &req, IResponse &res)
{
    std::string traceId = req.getTraceId();

    for (auto &h : handlers_)
    {
        try
        {
            std::string handlerName = h->name();
            auto start = std::chrono::steady_clock::now();

            logger_->log(LogLevel::Debug, "ChainHandler",
                         "[" + traceId + "] " + handlerName + " started");

            h->handle(req, res);

            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                               std::chrono::steady_clock::now() - start)
                               .count();

            logger_->log(LogLevel::Debug, "ChainHandler",
                         "[" + traceId + "] " + handlerName + " finished (" +
                             std::to_string(elapsed) + "ms) with status " +
                             std::to_string(res.getStatus()));
        }
        catch (const HttpError &e)
        {
            logger_->log(LogLevel::Error, "ChainHandler",
                         "[" + traceId + "] HttpError: " + std::to_string(e.statusCode()) + " - " + e.message());
            errorHandler_->handleError(res, e);
            res.setTraceId(traceId);
            return;
        }
        catch (const std::exception &e)
        {
            logger_->log(LogLevel::Error, "ChainHandler",
                         "[" + traceId + "] Unhandled exception: " + std::string(e.what()));
            errorHandler_->handleError(res, HttpError(500, "Internal server error"));
            res.setTraceId(traceId);
            return;
        }
    }

    if (res.getStatus() < 100 || res.getStatus() >= 600)
    {
        logger_->log(LogLevel::Error, "ChainHandler",
                     "[" + traceId + "] Chain finished with invalid HTTP status: " + std::to_string(res.getStatus()));
        errorHandler_->handleError(res, HttpError(500, "Internal server error"));
    }

    res.setTraceId(traceId);
}