#pragma once

/**
 * @file ITransactionExecutor.hpp
 * @brief Output port for transactional database operations
 * @author Anton Tobolkin
 */

/**
 * @class ITransactionExecutor
 * @brief Interface for executing operations within a database transaction
 *
 * Defines the contract for transaction management. The concrete
 * PostgresTransactionExecutor (microservice-boost) provides pqxx-specific
 * query<T>() and execute() methods that wrap pqxx::work in try/commit/catch
 * with error logging.
 *
 * Repositories depend on the concrete PostgresTransactionExecutor which
 * exposes the full pqxx-aware API. This interface serves as a type anchor
 * for DI wiring and future alternative implementations.
 */
class ITransactionExecutor {
public:
    virtual ~ITransactionExecutor() = default;
};
