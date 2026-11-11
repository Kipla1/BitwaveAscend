#include "payment/PaymentServiceImpl.h"

namespace bitwave::payment {

namespace {

shared::PurchaseResult failure(const std::string& message) {
    shared::PurchaseResult result;
    result.success = false;
    result.message = message;
    return result;
}

} // namespace

PaymentServiceImpl::PaymentServiceImpl(persistence::Database& db,
                                       Store& store,
                                       Wallet& wallet,
                                       player::Inventory& inventory,
                                       persistence::TransactionRepository& transactions,
                                       const auth::Session& session)
    : db_(db), store_(store), wallet_(wallet),
      inventory_(inventory), transactions_(transactions), session_(session) {}

shared::PurchaseResult PaymentServiceImpl::purchaseItem(std::int64_t itemId) {
    // Checked explicitly, even though Wallet/Inventory already guard this
    // internally — so the caller gets "you're not logged in" instead of a
    // generic "insufficient funds" that would be misleading here.
    if (!session_.isActive()) {
        return failure("You must be logged in to make a purchase.");
    }

    // Checked before beginTransaction(): there's nothing to roll back if
    // the item was never real to begin with.
    const auto item = store_.getItem(itemId);
    if (!item) {
        return failure("That item does not exist.");
    }

    if (item->kind == shared::StoreItemKind::Cosmetic) {
        if (item->price == 0)
            return failure("Free cosmetics can be equipped without purchase.");
        for (const auto& owned : inventory_.listItems())
            if (owned.itemId == itemId && owned.quantity > 0)
                return failure("You already own this cosmetic.");
    }

    db_.beginTransaction();
    try {
        if (!wallet_.spend(item->price)) {
            db_.rollbackTransaction();
            return failure("Insufficient funds.");
        }
        inventory_.addItem(itemId, 1);
        transactions_.record(session_.user()->id, itemId, item->price,
                             persistence::TransactionStatus::Succeeded);
        db_.commitTransaction();
    } catch (...) {
        try { db_.rollbackTransaction(); } catch (...) {}
        throw;
    }

    shared::PurchaseResult result;
    result.success = true;
    result.item = *item;
    result.amount = item->price;
    result.message = "Purchase successful.";
    return result;
}

} // namespace bitwave::payment
