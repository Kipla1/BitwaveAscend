#pragma once

#include "persistence/Database.h"
#include "persistence/WalletRepository.h"

namespace bitwave::persistence {

class SqliteWalletRepository : public WalletRepository {
public:
    explicit SqliteWalletRepository(Database& db);

    void create(std::int64_t userId) override;
    std::optional<long long> getBalance(std::int64_t userId) const override;
    bool spend(std::int64_t userId, long long amount) override;
    void addFunds(std::int64_t userId, long long amount) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence