#pragma once

#include "IEnvironment.hpp"
#include "IHttpHandler.hpp"
#include "ILogger.hpp"
#include "ChainHandler.hpp"
#include "NullLogger.hpp"
#include <memory>
#include <string>

class IWebApplication
{
public:
    IWebApplication() : logger_(std::make_shared<NullLogger>()) {}
    virtual ~IWebApplication() = default;

    virtual void run(int argc, char *argv[])
    {
        loadEnvironment(argc, argv);
        configureInjection();
        start();
    }

    void setLogger(std::shared_ptr<ILogger> logger)
    {
        logger_ = logger ? logger : std::make_shared<NullLogger>();
    }

    std::shared_ptr<ILogger> getLogger() const
    {
        return logger_;
    }

    template <typename... Handlers>
    void registerEndpoint(const std::string &method,
                          const std::string &pattern,
                          Handlers &&...handlers)
    {
        registerHandler(method, pattern,
                        std::make_shared<ChainHandler>(std::forward<Handlers>(handlers)...));
    }

protected:
    virtual void loadEnvironment(int argc, char *argv[]) = 0;
    virtual void configureInjection() = 0;
    virtual void start() = 0;

    virtual void registerHandler(
        const std::string &method,
        const std::string &pattern,
        std::shared_ptr<IHttpHandler> handler) = 0;

    std::shared_ptr<IEnvironment> env_;
    std::shared_ptr<ILogger> logger_;
};
