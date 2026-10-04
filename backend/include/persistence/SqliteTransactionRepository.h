#pragma once

#include "persistence/Database.h"
#include "persistence/TransactionRepository.h"

namespace bitwave::persistence {

class SqliteTransactionRepository : public TransactionRepository {
public:
    explicit SqliteTransactionRepository(Database& db);

    std::int64_t record(std::int64_t userId, std::int64_t itemId,
                        long long amount, TransactionStatus status) override;
    std::vector<TransactionRecord> listForUser(std::int64_t userId) const override;

private:
    Database& db_;
};

} // namespace bitwave::persistence