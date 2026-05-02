#pragma once

#include "application/IWebApplication.hpp"
#include "application/RouteTrie.hpp"
#include "ports/input/IHttpHandler.hpp"
#include "ports/output/ILogger.hpp"
#include "ports/output/IShutdown.hpp"
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
 * Provides handler registration, request routing (trie-based),
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
    /** @brief Trie-based route registry */
    RouteTrie trie_;
};