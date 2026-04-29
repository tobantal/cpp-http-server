#include "adapters/secondary/PostgresTransactionExecutor.hpp"

void PostgresTransactionExecutor::execute(pqxx::connection& conn,
                                          const std::string& context,
                                          std::function<void(pqxx::work&)> fn)
{
    try {
        pqxx::work txn(conn);
        fn(txn);
        txn.commit();
    } catch (const std::exception& e) {
        logger_->log(LogLevel::Error, "TransactionExecutor",
            "[" + context + "] failed: " + e.what());
        throw;
    }
}
