#pragma once

#include <vector>
#include "persistence/TransactionRepository.h"

namespace bitwave::persistence {

class InMemoryTransactionRepository : public TransactionRepository {
public:
    std::int64_t record(std::int64_t userId, std::int64_t itemId,
                        long long amount, TransactionStatus status) override {
        TransactionRecord r;
        r.id = nextId_++;
        r.userId = userId;
        r.itemId = itemId;
        r.amount = amount;
        r.timestamp = "test-timestamp"; // no real clock needed for tests
        r.status = status;
        records_.push_back(r);
        return r.id;
    }

    std::vector<TransactionRecord> listForUser(std::int64_t userId) const override {
        std::vector<TransactionRecord> result;
        for (const auto& r : records_) {
            if (r.userId == userId) result.push_back(r);
        }
        return result;
    }

private:
    std::vector<TransactionRecord> records_;
    std::int64_t nextId_ = 1;
};

} // namespace bitwave::persistence