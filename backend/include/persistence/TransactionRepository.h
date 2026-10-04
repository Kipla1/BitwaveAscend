#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace bitwave::persistence {

enum class TransactionStatus {
    Succeeded,
    Failed
};

struct TransactionRecord {
    std::int64_t      id = 0;
    std::int64_t      userId = 0;
    std::int64_t      itemId = 0;
    long long         amount = 0;
    std::string       timestamp;
    TransactionStatus status = TransactionStatus::Succeeded;
};

class TransactionRepository {
public:
    virtual ~TransactionRepository() = default;

    // Returns the new transaction's id.
    virtual std::int64_t record(std::int64_t userId, std::int64_t itemId,
                                long long amount, TransactionStatus status) = 0;

    virtual std::vector<TransactionRecord> listForUser(std::int64_t userId) const = 0;
};

} // namespace bitwave::persistence