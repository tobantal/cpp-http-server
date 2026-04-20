#pragma once

#include "IEnvironment.hpp"
#include "IHttpHandler.hpp"
#include <memory>
#include <string>

class IWebApplication
{
public:
    virtual ~IWebApplication() = default;

    virtual void run(int argc, char *argv[])
    {
        loadEnvironment(argc, argv);
        configureInjection();
        start();
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
};
