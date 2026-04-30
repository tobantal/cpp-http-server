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
    std::string generate() override;
};