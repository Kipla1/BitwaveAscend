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

    db_.beginTransaction();

    if (!wallet_.spend(item->price)) {
        db_.rollbackTransaction();
        return failure("Insufficient funds.");
    }

    try {
        inventory_.addItem(itemId, 1);
        transactions_.record(session_.user()->id, itemId, item->price,
                             persistence::TransactionStatus::Succeeded);
    } catch (...) {
        // Something unexpected failed mid-purchase (e.g. a database error).
        // Undo the wallet debit along with everything else, then let the
        // caller see the real error rather than silently losing it.
        db_.rollbackTransaction();
        throw;
    }

    db_.commitTransaction();

    shared::PurchaseResult result;
    result.success = true;
    result.item = *item;
    result.amount = item->price;
    result.message = "Purchase successful.";
    return result;
}

} // namespace bitwave::payment