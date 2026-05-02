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

/**
 * @enum ServerState
 * @brief Server lifecycle states
 */
enum class ServerState : uint8_t
{
    NotStarted,
    Running,
    Stopped
};

/**
 * @class BaseWebApplication
 * @brief Boost-independent base class for HTTP server applications
 *
 * Provides handler registration, request routing (exact + wildcard),
 * and error handling (HttpError, std::exception). Boost-specific
 * transport logic (io_context, acceptor, sessions) lives in
 * BoostBeastApplication, which inherits from this class.
 */
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
    /** @brief Application logger */
    std::shared_ptr<ILogger> logger_;
    /** @brief Atomic server state (NotStarted → Running → Stopped) */
    std::atomic<ServerState> state_{ServerState::NotStarted};

private:
    /** @brief Match result: handler + matched pattern */
    struct HandlerMatch
    {
        std::shared_ptr<IHttpHandler> handler;
        std::string pattern;
    };

    /** @brief Route registry: pattern → HTTP method → handler */
    std::map<std::string, std::map<std::string, std::shared_ptr<IHttpHandler>>> handlers_;

    /** @brief Find handler by method and path (exact match first, then wildcard) */
    std::optional<HandlerMatch> findHandler(const std::string &method, const std::string &path);
    /** @brief Check if any handler is registered for the given path */
    bool pathExists(const std::string &path);
    /** @brief Collect HTTP methods allowed for the given path (for 405 Allow header) */
    std::vector<std::string> getAllowedMethods(const std::string &path);
    /** @brief Check if pattern contains dynamic segments (* or :param) */
    static bool hasParameters(const std::string &pattern);
};