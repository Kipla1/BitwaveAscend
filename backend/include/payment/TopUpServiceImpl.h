#pragma once

#include "auth/Session.h"
#include "payment/PaymentGateway.h"
#include "payment/TopUpService.h"
#include "payment/Wallet.h"
#include "persistence/Database.h"
#include "persistence/PaymentOrderRepository.h"

namespace bitwave::payment {

class TopUpServiceImpl : public TopUpService {
public:
    // `db` must be the same Database the orders repository and wallet write
    // through, or the transaction in confirmTopUp won't cover them.
    TopUpServiceImpl(persistence::Database& db,
                     PaymentGateway& gateway,
                     persistence::PaymentOrderRepository& orders,
                     Wallet& wallet,
                     const auth::Session& session);

    TopUpStart startTopUp(const std::string& email, int amountKes) override;
    TopUpResult confirmTopUp(const std::string& reference) override;

private:
    persistence::Database& db_;
    PaymentGateway& gateway_;
    persistence::PaymentOrderRepository& orders_;
    Wallet& wallet_;
    const auth::Session& session_;
};

} // namespace bitwave::payment