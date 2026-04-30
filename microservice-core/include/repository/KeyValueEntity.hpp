#pragma once

#include <string>

/**
 * @file KeyValueEntity.hpp
 * @brief Simple key-value entity for repository operations
 * @author Anton Tobolkin
 */

/**
 * @struct KeyValueEntity
 * @brief A simple key-value pair for persistence
 */
struct KeyValueEntity
{
    std::string id;   ///< Key (used as ID)
    std::string value; ///< JSON value
};