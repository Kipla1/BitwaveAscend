#pragma once

#include <cstdint>

namespace bitwave::payment {

class Wallet {
public:
    virtual ~Wallet() = default;

    // Balance for the currently logged-in player. 0 if nobody is logged in.
    virtual long long getBalance() const = 0;

    // True if the spend succeeded. False if insufficient funds, or no one
    // is logged in — the caller can't tell which from the return value
    // alone; see PaymentService (Day 10) for the user-facing distinction.
    virtual bool spend(long long amount) = 0;

    virtual void addFunds(long long amount) = 0;
};

} // namespace bitwave::payment