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

    handlers_[pattern][method] = handler;

    logger_->log(LogLevel::Info, "App",
                 "Registered: " + method + " " + pattern);
}

std::optional<BaseWebApplication::HandlerMatch> BaseWebApplication::findHandler(
    const std::string &method,
    const std::string &path)
{
    auto exactIt = handlers_.find(path);
    if (exactIt != handlers_.end())
    {
        auto methodIt = exactIt->second.find(method);
        if (methodIt != exactIt->second.end())
        {
            return HandlerMatch{methodIt->second, path};
        }
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (!hasParameters(pattern))
        {
            continue;
        }

        if (pattern.find(':') != std::string::npos && RouteMatcher::matches(pattern, path))
        {
            auto methodIt = methodHandlers.find(method);
            if (methodIt != methodHandlers.end())
            {
                return HandlerMatch{methodIt->second, pattern};
            }
        }
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (!hasParameters(pattern))
        {
            continue;
        }

        if (pattern.find('*') != std::string::npos && RouteMatcher::matches(pattern, path))
        {
            auto methodIt = methodHandlers.find(method);
            if (methodIt != methodHandlers.end())
            {
                return HandlerMatch{methodIt->second, pattern};
            }
        }
    }

    return std::nullopt;
}

bool BaseWebApplication::pathExists(const std::string &path)
{
    if (handlers_.find(path) != handlers_.end())
    {
        return true;
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (hasParameters(pattern) && RouteMatcher::matches(pattern, path))
        {
            return true;
        }
    }

    return false;
}

std::vector<std::string> BaseWebApplication::getAllowedMethods(const std::string &path)
{
    auto exactIt = handlers_.find(path);
    if (exactIt != handlers_.end())
    {
        std::vector<std::string> methods;
        for (const auto &[method, _] : exactIt->second)
            methods.push_back(method);
        return methods;
    }

    for (const auto &[pattern, methodHandlers] : handlers_)
    {
        if (hasParameters(pattern) && RouteMatcher::matches(pattern, path))
        {
            std::vector<std::string> methods;
            for (const auto &[method, _] : methodHandlers)
                methods.push_back(method);
            return methods;
        }
    }

    return {};
}

bool BaseWebApplication::hasParameters(const std::string &pattern)
{
    return pattern.find('*') != std::string::npos || pattern.find(':') != std::string::npos;
}

void BaseWebApplication::handleRequest(IRequest &req, IResponse &res)
{
    std::string path = req.getPath();
    std::string method = req.getMethod();

    logger_->log(LogLevel::Info, "App",
                 method + " " + path + " from " + req.getIp());

    auto match = findHandler(method, path);

    if (match)
    {
        try
        {
            req.setPathPattern(match->pattern);
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
        if (pathExists(path))
        {
            auto allowed = getAllowedMethods(path);
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