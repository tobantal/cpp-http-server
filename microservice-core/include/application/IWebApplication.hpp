#pragma once

#include "ports/output/IEnvironment.hpp"
#include "ports/input/IHttpHandler.hpp"
#include <memory>
#include <string>

/**
 * @file IWebApplication.hpp
 * @brief Web application interface
 * @author Anton Tobolkin
 */

/**
 * @class IWebApplication
 * @brief Interface for web application lifecycle
 */
class IWebApplication
{
public:
    virtual ~IWebApplication() = default;

    /**
     * @brief Run the application
     * @param argc Argument count
     * @param argv Argument values
     */
    virtual void run(int argc, char *argv[])
    {
        loadEnvironment(argc, argv);
        configureInjection();
        start();
    }

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
     * @brief Register an HTTP handler
     * @param method HTTP method
     * @param pattern URL pattern
     * @param handler HTTP handler
     */
    virtual void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) = 0;

    std::shared_ptr<IEnvironment> env_;
};
