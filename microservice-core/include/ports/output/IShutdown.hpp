#pragma once

#include "domain/INameable.hpp"
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
 * Inherits INameable for logging (component name in ShutdownManager logs).
 */
struct IShutdown : public INameable
{
    /**
     * @brief Shut down the component gracefully
     * @param timeoutMs Maximum time in milliseconds to wait for shutdown.
     *                   0 means no timeout (wait indefinitely).
     *                   Implementation should respect this timeout.
     */
    virtual void shutdown(std::chrono::milliseconds timeoutMs = std::chrono::milliseconds(5000)) = 0;
};