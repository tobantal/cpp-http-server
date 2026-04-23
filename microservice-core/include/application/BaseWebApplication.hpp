#pragma once

#include "application/IWebApplication.hpp"
#include "ports/input/IHttpHandler.hpp"
#include "ports/output/ILogger.hpp"
#include "ports/output/IShutdown.hpp"
#include "adapters/primary/RouteMatcher.hpp"
#include "application/ChainHandler.hpp"
#include "domain/error/HttpError.hpp"
#include "util/StringUtils.hpp"
#include <memory>
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <atomic>
#include <cstdint>

enum class ServerState : uint8_t
{
    NotStarted,
    Running,
    Stopped
};

class BaseWebApplication : public IWebApplication, public IShutdown
{
public:
    explicit BaseWebApplication(std::shared_ptr<ILogger> logger);
    virtual ~BaseWebApplication();

    void stop() override;
    void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) override;

    template <typename... Handlers>
    void registerEndpoint(const std::string &method,
                          const std::string &pattern,
                          Handlers &&...handlers)
    {
        registerHandler(method, pattern,
                        std::make_shared<ChainHandler>(logger_, std::forward<Handlers>(handlers)...));
    }

    void handleRequest(IRequest &req, IResponse &res);

protected:
    void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) override;

    std::shared_ptr<ILogger> logger_;
    std::atomic<ServerState> state_{ServerState::NotStarted};

private:
    struct HandlerMatch
    {
        std::shared_ptr<IHttpHandler> handler;
        std::string pattern;
    };

    std::map<std::string, std::map<std::string, std::shared_ptr<IHttpHandler>>> handlers_;

    std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);
    bool pathExists(const std::string &path);
    std::vector<std::string> getAllowedMethods(const std::string &path);
};