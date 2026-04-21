#include "application/IWebApplication.hpp"
#include <iostream>

/**
 * @file IWebApplication.cpp
 * @brief IWebApplication implementation
 * @author Anton Tobolkin
 */

IWebApplication *IWebApplication::instance_ = nullptr;

int IWebApplication::run(int argc, char *argv[])
{
    try
    {
        instance_ = this;
        loadEnvironment(argc, argv);
        configureInjection();
        installSignalHandlers();
        start();
        instance_ = nullptr;
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "[main] Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

void IWebApplication::installSignalHandlers()
{
    std::signal(SIGINT, IWebApplication::signalHandler);
    std::signal(SIGTERM, IWebApplication::signalHandler);
}

void IWebApplication::signalHandler(int signal)
{
    if (instance_)
    {
        std::cout << "\n[main] Received signal " << signal << ", shutting down..." << std::endl;
        instance_->stop();
    }
}