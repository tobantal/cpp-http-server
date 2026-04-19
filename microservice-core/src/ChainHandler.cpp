#include "ChainHandler.hpp"

void ChainHandler::handle(IRequest &req, IResponse &res)
{
    for (auto &h : handlers_)
    {
        try
        {
            h->handle(req, res);
        }
        catch (const HttpError &e)
        {
            sendError(res, e.statusCode(), e.message());
            return;
        }
        catch (const std::exception &e)
        {
            if (logger_)
            {
                logger_->log(LogLevel::Error, "ChainHandler",
                             std::string("Unhandled exception: ") + e.what());
            }
            sendError(res, 500, "Internal server error");
            return;
        }
    }

    if (res.getStatus() < 100 || res.getStatus() >= 600)
    {
        if (logger_)
        {
            logger_->log(LogLevel::Error, "ChainHandler",
                         "Chain finished with invalid HTTP status: " + std::to_string(res.getStatus()));
        }
        sendError(res, 500, "Internal server error");
    }
}

void ChainHandler::sendError(IResponse &res, int status, const std::string &message)
{
    res.setResult(status, "application/json",
                  R"({"error": ")" + StringUtils::escapeJson(message) + R"("})");
}