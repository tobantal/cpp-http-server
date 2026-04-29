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
    /**
     * @brief Construct ShutdownManager
     * @param logger Logger instance
     * @param defaultTimeout Default timeout per component
     */
    explicit ShutdownManager(std::shared_ptr<ILogger> logger = std::make_shared<NullLogger>(),
                              std::chrono::milliseconds defaultTimeout = std::chrono::milliseconds(5000))
        : logger_(std::move(logger)), defaultTimeout_(defaultTimeout) {}

    /**
     * @brief Register a component for shutdown
     * @param component Component implementing IShutdown
     */
    void registerComponent(std::shared_ptr<IShutdown> component);

    /**
     * @brief Shut down all registered components in LIFO order
     *
     * Calls shutdown() on each component in reverse registration order.
     * If a component's shutdown exceeds its timeout, logs a warning and continues.
     */
    void shutdownAll();

    /**
     * @brief Get number of registered components
     * @return Number of components
     */
    size_t size() const;

private:
    /** @brief Logger instance */
    std::shared_ptr<ILogger> logger_;

    /** @brief Default timeout per component in milliseconds */
    std::chrono::milliseconds defaultTimeout_;

    /** @brief Registered components */
    std::vector<std::shared_ptr<IShutdown>> components_;

    /** @brief Mutex for thread-safe access */
    mutable std::mutex mutex_;
};