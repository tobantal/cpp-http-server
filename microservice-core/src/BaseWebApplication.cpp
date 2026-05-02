#include "application/BaseWebApplication.hpp"
#include "application/ChainHandler.hpp"
#include "domain/error/MethodNotAllowedError.hpp"

BaseWebApplication::BaseWebApplication(std::shared_ptr<ILogger> logger)
    : logger_(std::move(logger))
{
    logger_->log(LogLevel::Info, "App", "BaseWebApplication created");
}

BaseWebApplication::~BaseWebApplication()
{
    stop();
    logger_->log(LogLevel::Info, "App", "BaseWebApplication destroyed");
}

void BaseWebApplication::stop()
{
    ServerState expected = ServerState::Running;
    if (state_.compare_exchange_strong(expected, ServerState::Stopped))
    {
        logger_->log(LogLevel::Info, "App", "Stopping application...");
    }
}

void BaseWebApplication::shutdown(std::chrono::milliseconds /*timeoutMs*/)
{
    stop();
}

void BaseWebApplication::registerHandler(
    const std::string &method,
    const std::string &pattern,
    std::shared_ptr<IHttpHandler> handler)
{
    if (state_.load() != ServerState::NotStarted)
    {
        throw std::logic_error("Cannot register handler after server has started");
    }

    trie_.insert(pattern, method, handler);

    logger_->log(LogLevel::Info, "App",
                 "Registered: " + method + " " + pattern);
}

void BaseWebApplication::handleRequest(IRequest &req, IResponse &res)
{
    std::string path = req.getPath();
    std::string method = req.getMethod();

    logger_->log(LogLevel::Info, "App",
                 method + " " + path + " from " + req.getIp());

    auto match = trie_.lookup(method, path);

    if (match)
    {
        try
        {
            req.setPathPattern(match->pattern);
            req.setPathParams(match->pathParams);
            match->handler->handle(req, res);
        }
        catch (const HttpError &e)
        {
            logger_->log(LogLevel::Error, "App",
                         "HttpError: " + std::to_string(e.statusCode()) + " - " + e.message());
            res.setResult(e.statusCode(), "application/json",
                          R"({"error": ")" + StringUtils::escapeJson(e.message()) + R"("})");
        }
        catch (const std::exception &e)
        {
            logger_->log(LogLevel::Error, "App",
                         std::string("Handler error: ") + e.what());
            res.setResult(500, "application/json", "{\"error\": \"Internal server error\"}");
        }
    }
    else
    {
        if (trie_.lookupAny(path))
        {
            auto allowed = trie_.lookupMethods(path);
            std::string allowValue;
            for (size_t i = 0; i < allowed.size(); ++i)
            {
                if (i > 0) allowValue += ", ";
                allowValue += allowed[i];
            }
            res.setHeader("Allow", allowValue);
            res.setResult(405, "application/json",
                          R"({"error": ")" + StringUtils::escapeJson(
                              "Method " + method + " not allowed for " + path) + R"("})");
        }
        else
        {
            logger_->log(LogLevel::Warn, "App", "No handler found");
            res.setResult(404, "application/json", "{\"error\": \"Not found\"}");
        }
    }
}
