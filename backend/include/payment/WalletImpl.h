#pragma once

#include "auth/Session.h"
#include "payment/Wallet.h"
#include "persistence/WalletRepository.h"

namespace bitwave::payment {

class WalletImpl : public Wallet {
public:
    WalletImpl(persistence::WalletRepository& repo, const auth::Session& session);

    long long getBalance() const override;
    bool spend(long long amount) override;
    void addFunds(long long amount) override;

private:
    persistence::WalletRepository& repo_;
    const auth::Session& session_; // read-only: Wallet never logs anyone in or out
};

} // namespace bitwave::payment