/**
 * @file UuidGenerator.cpp
 * @brief Thread-safe UUID generator implementation
 * @author Anton Tobolkin
 */

#include "util/UuidGenerator.hpp"
#include <chrono>
#include <cstdint>
#include <random>
#include <sstream>
#include <iomanip>

std::string UuidGenerator::generate()
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
