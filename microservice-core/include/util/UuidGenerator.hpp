#pragma once

#include "util/IIdGenerator.hpp"
#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>

/**
 * @file UuidGenerator.hpp
 * @brief Thread-safe UUID generator implementation
 * @author Anton Tobolkin
 *
 * WARNING: This generator does NOT produce RFC 4122/RFC 9562 compliant UUIDs.
 * The output is a 32-character hex string (128 bits: timestamp XOR random + counter XOR random)
 * without version nibble, variant bits, or hyphens.
 * For RFC-compliant UUIDs, consider migrating to UUID v7 (see SRV-44 in v0.4.0 backlog).
 */

/**
 * @class UuidGenerator
 * @brief Thread-safe ID generator using thread_local Mersenne Twister
 *
 * Algorithm: 128-bit output = hi(64 bits) + lo(64 bits)
 *   hi = timestamp_ns XOR (random << 32)
 *   lo = counter XOR random
 *
 * Uses thread_local for zero-contention performance.
 * Each thread gets its own RNG, counter, and distribution.
 */
class UuidGenerator : public IIdGenerator
{
public:
    /**
     * @brief Generate a UUID string
     * @return 32-character hex string (128 bits)
     */
    std::string generate() override;
};