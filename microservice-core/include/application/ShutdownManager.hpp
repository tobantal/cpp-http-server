#pragma once

#include "ports/output/IShutdown.hpp"
#include "ports/output/ILogger.hpp"
#include "util/Timer.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <vector>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <memory>

/**
 * @file ShutdownManager.hpp
 * @brief Manages graceful shutdown of registered components in LIFO order
 * @author Anton Tobolkin
 */

/**
 * @class ShutdownManager
 * @brief Registers IShutdown components and shuts them down in reverse order
 *
 * Components are shut down in LIFO order (last registered = first shut down).
 * Each component gets a configurable timeout. If timeout expires, a warning is
 * logged and shutdown continues with the next component.
 */
class ShutdownManager
{
public:
    explicit ShutdownManager(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>(),
                              std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(5000))
        : logger_(std::move(logger)), defaultTimeout_(defaultTimeout) {}

    /**
     * @brief Register a component for shutdown
     * @param component Component implementing IShutdown
     */
    void registerComponent(std::shared_ptr<IShutdown> component)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        components_.push_back(component);
    }

    /**
     * @brief Shut down all registered components in LIFO order
     *
     * Calls shutdown() on each component in reverse registration order.
     * If a component's shutdown exceeds its timeout, logs a warning and continues.
     */
    void shutdownAll()
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

    /**
     * @brief Get number of registered components
     * @return Number of components
     */
    size_t size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return components_.size();
    }

private:
    std::shared_ptr<ILogger> logger_;
    std::chrono::milliseconds defaultTimeout_;
    std::vector<std::shared_ptr<IShutdown>> components_;
    mutable std::mutex mutex_;
};