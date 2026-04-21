#pragma once

#include <string>
#include <chrono>

/**
 * @file IShutdown.hpp
 * @brief Interface for graceful shutdown participants
 * @author Anton Tobolkin
 */

/**
 * @struct IShutdown
 * @brief Interface for components that need graceful shutdown
 *
 * Implement this interface for any component that needs to be
 * stopped in a controlled manner during application shutdown.
 * Components are shut down in reverse order of registration (LIFO).
 */
struct IShutdown
{
    virtual ~IShutdown() = default;

    /**
     * @brief Shut down the component gracefully
     *
     * Implementation should:
     * - Stop accepting new work
     * - Finish in-progress work
     * - Release resources
     *
     * @param timeoutMs Maximum time in milliseconds to wait for shutdown.
     *                   0 means no timeout (wait indefinitely).
     *                   Implementation should respect this timeout.
     */
    virtual void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) = 0;

    /**
     * @brief Get component name for logging
     * @return Component name
     */
    virtual std::string name() const = 0;
};