#pragma once

#include "repository/IRepository.hpp"
#include "adapters/secondary/ConnectionPool.hpp"
#include "adapters/secondary/PostgresTransactionExecutor.hpp"
#include "ports/output/ILogger.hpp"
#include "adapters/secondary/NullLogger.hpp"
#include <pqxx/pqxx>
#include <memory>
#include <string>
#include <optional>
#include <vector>

/**
 * @file KeyValueEntity.hpp
 * @brief Simple key-value entity for PostgresKeyValueRepository
 * @author Anton Tobolkin
 */

/**
 * @struct KeyValueEntity
 * @brief A simple key-value pair stored in the database
 */
struct KeyValueEntity
{
    std::string id;   ///< Key (used as ID)
    std::string value; ///< JSON value
};