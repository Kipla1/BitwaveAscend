#pragma once

#include <cstdint>
#include <optional>

namespace bitwave::persistence {

class WalletRepository {
public:
    virtual ~WalletRepository() = default;

    // Creates the row with a zero balance. Called once, at registration.
    virtual void create(std::int64_t userId) = 0;

    virtual std::optional<long long> getBalance(std::int64_t userId) const = 0;

    // True if the spend succeeded (sufficient funds). False otherwise.
    virtual bool spend(std::int64_t userId, long long amount) = 0;

    virtual void addFunds(std::int64_t userId, long long amount) = 0;
};

} // namespace bitwave::persistence