#pragma once

#include <string>
#include <memory>

/**
 * @file IIdGenerator.hpp
 * @brief Interface for ID generation (Dependency Inversion Principle)
 * @author Anton Tobolkin
 */

/**
 * @struct IIdGenerator
 * @brief Interface for generating unique identifiers
 *
 * Follows SOLID/DIP: high-level modules depend on this abstraction,
 * not on a concrete generator. This allows injecting deterministic
 * IDs in tests (mock/stub) and swapping implementations (UUID v4, v7, ULID).
 */
struct IIdGenerator
{
    virtual ~IIdGenerator() = default;

    /**
     * @brief Generate a unique identifier string
     * @return Unique ID string (format depends on implementation)
     */
    virtual std::string generate() = 0;
};