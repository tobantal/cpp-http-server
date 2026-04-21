#pragma once

#include "ports/output/IEnvironment.hpp"
#include "ports/input/IHttpHandler.hpp"
#include <memory>
#include <string>
#include <csignal>

/**
 * @file IWebApplication.hpp
 * @brief Web application interface
 * @author Anton Tobolkin
 */

/**
 * @class IWebApplication
 * @brief Interface for web application lifecycle
 *
 * Provides a Template Method pattern: run() calls loadEnvironment(),
 * configureInjection(), installSignalHandlers(), then start().
 * Signal handlers (SIGINT/SIGTERM) are installed by default,
 * triggering graceful shutdown via IShutdown.
 */
class IWebApplication
{
public:
    virtual ~IWebApplication() = default;

    /**
     * @brief Run the application with signal handling and error catching
     * @param argc Argument count
     * @param argv Argument values
     * @return Exit code (0 = success, 1 = error)
     *
     * Installs SIGINT/SIGTERM handlers, then calls loadEnvironment(),
     * configureInjection(), start(). On exception, logs and returns 1.
     */
    virtual int run(int argc, char *argv[]);

protected:
    /**
     * @brief Load environment configuration
     * @param argc Argument count
     * @param argv Argument values
     */
    virtual void loadEnvironment(int argc, char *argv[]) = 0;

    /**
     * @brief Configure dependency injection
     */
    virtual void configureInjection() = 0;

    /**
     * @brief Start the application
     */
    virtual void start() = 0;

    /**
     * @brief Stop the application gracefully
     *
     * Called by the default signal handler on SIGINT/SIGTERM.
     * Override to customize shutdown behavior.
     */
    virtual void stop() = 0;

    /**
     * @brief Register an HTTP handler
     * @param method HTTP method
     * @param pattern URL pattern
     * @param handler HTTP handler
     */
    virtual void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) = 0;

    /**
     * @brief Install signal handlers for graceful shutdown
     *
     * Default implementation registers SIGINT/SIGTERM that call
     * the application's stop/shutdown method. Override for custom behavior.
     */
    virtual void installSignalHandlers();

    std::shared_ptr<IEnvironment> env_;

private:
    static IWebApplication *instance_;
    static void signalHandler(int signal);
};
