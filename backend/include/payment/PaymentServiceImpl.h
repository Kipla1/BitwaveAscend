#pragma once

#include "auth/Session.h"
#include "payment/PaymentService.h"
#include "payment/Store.h"
#include "payment/Wallet.h"
#include "persistence/Database.h"
#include "persistence/TransactionRepository.h"
#include "player/Inventory.h"

namespace bitwave::payment {

class PaymentServiceImpl : public PaymentService {
public:
    // All references must outlive this object. `db` must be the same
    // Database that `wallet`/`inventory`'s repositories write through,
    // or the transaction below won't actually cover their writes.
    PaymentServiceImpl(persistence::Database& db,
                       Store& store,
                       Wallet& wallet,
                       player::Inventory& inventory,
                       persistence::TransactionRepository& transactions,
                       const auth::Session& session);

    shared::PurchaseResult purchaseItem(std::int64_t itemId) override;

private:
    persistence::Database& db_;
    Store& store_;
    Wallet& wallet_;
    player::Inventory& inventory_;
    persistence::TransactionRepository& transactions_;
    const auth::Session& session_;
};

} // namespace bitwave::payment
