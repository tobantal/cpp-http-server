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
    std::string generate() override
    {
        thread_local std::mt19937_64 rng(std::random_device{}());
        thread_local uint64_t counter = 0;
        thread_local std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        ++counter;

        uint64_t rnd = dist(rng);
        uint64_t hi = static_cast<uint64_t>(now) ^ (rnd << 32);
        uint64_t lo = counter ^ rnd;

        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(16) << hi
           << std::setw(16) << lo;
        return ss.str();
    }
};