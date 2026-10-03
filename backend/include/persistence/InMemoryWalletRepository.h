#pragma once

#include <unordered_map>
#include "persistence/WalletRepository.h"

namespace bitwave::persistence {

class InMemoryWalletRepository : public WalletRepository {
public:
    void create(std::int64_t userId) override {
        balances_[userId] = 0;
    }

    std::optional<long long> getBalance(std::int64_t userId) const override {
        auto it = balances_.find(userId);
        if (it == balances_.end()) return std::nullopt;
        return it->second;
    }

    bool spend(std::int64_t userId, long long amount) override {
        auto it = balances_.find(userId);
        if (it == balances_.end() || it->second < amount) return false;
        it->second -= amount;
        return true;
    }

    void addFunds(std::int64_t userId, long long amount) override {
        auto it = balances_.find(userId);
        if (it != balances_.end()) it->second += amount;
    }

private:
    std::unordered_map<std::int64_t, long long> balances_;
};

} // namespace bitwave::persistence