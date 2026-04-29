/**
 * @file ShutdownManager.cpp
 * @brief Manages graceful shutdown of registered components in LIFO order
 * @author Anton Tobolkin
 */

#include "application/ShutdownManager.hpp"

void ShutdownManager::registerComponent(std::shared_ptr<IShutdown> component)
{
    std::lock_guard<std::mutex> lock(mutex_);
    components_.push_back(component);
}

void ShutdownManager::shutdownAll()
{
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->log(LogLevel::Info, "ShutdownManager",
                 "Starting graceful shutdown of " + std::to_string(components_.size()) + " components...");

    for (auto it = components_.rbegin(); it != components_.rend(); ++it)
    {
        auto &component = *it;
        logger_->log(LogLevel::Info, "ShutdownManager",
                     "Shutting down " + component->name() + "...");

        Timer timer;
        timer.start();
        component->shutdown(defaultTimeout_);
        timer.stop();

        if (timer.elapsed() >= defaultTimeout_.count())
        {
            logger_->log(LogLevel::Warn, "ShutdownManager",
                         component->name() + " shutdown timed out after " + timer.show());
        }
        else
        {
            logger_->log(LogLevel::Info, "ShutdownManager",
                         component->name() + " shut down in " + timer.show());
        }
    }

    logger_->log(LogLevel::Info, "ShutdownManager", "All components shut down");
}

size_t ShutdownManager::size() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return components_.size();
}
