#include "handler/ChainHandler.hpp"

void ChainHandler::handle(IRequest &req, IResponse &res)
{
    std::string traceId = req.getTraceId();

    for (auto &h : handlers_)
    {
        try
        {
            h->handle(req, res);
        }
        catch (const HttpError &e)
        {
            logger_->log(LogLevel::Error, "ChainHandler",
                         "[" + traceId + "] HttpError: " + std::to_string(e.statusCode()) + " - " + e.message());
            sendError(res, e.statusCode(), e.message());
            res.setTraceId(traceId);
            return;
        }
        catch (const std::exception &e)
        {
            logger_->log(LogLevel::Error, "ChainHandler",
                         "[" + traceId + "] Unhandled exception: " + std::string(e.what()));
            sendError(res, 500, "Internal server error");
            res.setTraceId(traceId);
            return;
        }
    }

    if (res.getStatus() < 100 || res.getStatus() >= 600)
    {
        logger_->log(LogLevel::Error, "ChainHandler",
                     "[" + traceId + "] Chain finished with invalid HTTP status: " + std::to_string(res.getStatus()));
        sendError(res, 500, "Internal server error");
    }

    res.setTraceId(traceId);
}

void ChainHandler::sendError(IResponse &res, int status, const std::string &message)
{
    res.setResult(status, "application/json",
                  R"({"error": ")" + StringUtils::escapeJson(message) + R"("})");
}