#pragma once

#include "util/IIdGenerator.hpp"
#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>
#include <string>
#include <array>

/**
 * @file Uuid7Generator.hpp
 * @brief RFC 9562 UUIDv7 generator implementation
 * @author Anton Tobolkin
 *
 * Generates UUIDs conforming to RFC 9562 (UUIDv7):
 * - 48-bit millisecond Unix timestamp
 * - 74 bits of randomness (with version/variant bits set per spec)
 *
 * Output format: xxxxxxxx-xxxx-7xxx-yxxx-xxxxxxxxxxxx
 * where y ∈ {8,9,a,b} (variant 2 per RFC 4122/9562)
 *
 * Thread-safe via thread_local RNG.
 */

class Uuid7Generator : public IIdGenerator
{
public:
    std::string generate() override
    {
        thread_local std::mt19937_64 rng(std::random_device{}());
        thread_local std::uniform_int_distribution<uint64_t> dist(0, UINT64_MAX);

        auto now = std::chrono::system_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()).count();

        uint64_t rnd_a = dist(rng);
        uint64_t rnd_b = dist(rng);

        // Layout per RFC 9562:
        //   48 bits: unix_ts_ms
        //   4 bits:  version (0111 = 7)
        //   12 bits: rand_a
        //   2 bits:  variant (10)
        //   62 bits: rand_b

        // rnd_a: set version nibble to 7, keep lower 12 bits random
        uint64_t rand_a = rnd_a & 0x0FFFULL;
        uint64_t versioned_a = (static_cast<uint64_t>(ms) << 16) | (0x7ULL << 12) | rand_a;

        // rnd_b: set variant bits to 10xx, keep lower 62 bits random
        uint64_t rand_b = rnd_b & 0x3FFFFFFFFFFFFFFFULL;
        uint64_t versioned_b = (0x2ULL << 62) | rand_b;

        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(8) << ((versioned_a >> 32) & 0xFFFFFFFFULL) << "-"
           << std::setw(4) << ((versioned_a >> 16) & 0xFFFFULL) << "-"
           << std::setw(4) << (versioned_a & 0xFFFFULL) << "-"
           << std::setw(4) << ((versioned_b >> 48) & 0xFFFFULL) << "-"
           << std::setw(12) << (versioned_b & 0xFFFFFFFFFFFFULL);

        return ss.str();
    }
};