#pragma once

#include <vector>
#include <string>
#include <optional>
#include <memory>

/**
 * @file IRepository.hpp
 * @brief Generic repository interface for CRUD operations
 * @author Anton Tobolkin
 */

/**
 * @class IRepository
 * @brief Generic repository interface for entity persistence
 *
 * Provides a common CRUD contract for all repository types.
 * Concrete implementations (PostgresUserRepository, etc.) extend this
 * with domain-specific query methods.
 *
 * @tparam Entity The entity type managed by this repository
 */
template <typename Entity>
class IRepository
{
public:
    virtual ~IRepository() = default;

    /**
     * @brief Find an entity by its ID
     * @param id Entity identifier
     * @return The entity if found, std::nullopt otherwise
     */
    virtual std::optional<Entity> findById(const std::string &id) = 0;

    /**
     * @brief Find all entities
     * @return Vector of all entities
     */
    virtual std::vector<Entity> findAll() = 0;

    /**
     * @brief Save (insert or update) an entity
     * @param entity The entity to save
     * @return The saved entity (with generated ID if new)
     */
    virtual Entity save(const Entity &entity) = 0;

    /**
     * @brief Save multiple entities in a batch (insert or update)
     * @param entities Vector of entities to save
     * @return Vector of saved entities (with generated IDs if new)
     */
    virtual std::vector<Entity> saveAll(const std::vector<Entity> &entities) = 0;

    /**
     * @brief Remove an entity by its ID
     * @param id Entity identifier
     * @return true if the entity was removed, false if not found
     */
    virtual bool removeById(const std::string &id) = 0;
};