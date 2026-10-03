#include "payment/WalletImpl.h"

namespace bitwave::payment {

WalletImpl::WalletImpl(persistence::WalletRepository& repo, const auth::Session& session)
    : repo_(repo), session_(session) {}

long long WalletImpl::getBalance() const {
    if (!session_.isActive()) return 0;
    return repo_.getBalance(session_.user()->id).value_or(0);
}

bool WalletImpl::spend(long long amount) {
    if (!session_.isActive()) return false;
    return repo_.spend(session_.user()->id, amount);
}

void WalletImpl::addFunds(long long amount) {
    if (!session_.isActive()) return;
    repo_.addFunds(session_.user()->id, amount);
}

} // namespace bitwave::payment