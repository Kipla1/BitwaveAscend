#pragma once

#include "persistence/Database.h"
#include "persistence/PaymentOrderRepository.h"

namespace bitwave::persistence {

class SqlitePaymentOrderRepository : public PaymentOrderRepository {
public:
    // Needs the users table to exist first (foreign key).
    explicit SqlitePaymentOrderRepository(Database& db);

    void create(std::int64_t userId, const std::string& reference,
                long long amountCents, const std::string& currency,
                long long coins) override;
    std::optional<PaymentOrder> findByReference(const std::string& reference) const override;
    bool markSucceeded(const std::string& reference) override;
    bool markFailed(const std::string& reference) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence