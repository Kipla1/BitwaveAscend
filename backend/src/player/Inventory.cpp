#include "player/Inventory.h"

namespace bitwave::player {

InventoryImpl::InventoryImpl(persistence::InventoryRepository& repo, const auth::Session& session)
    : repo_(repo), session_(session) {}

std::vector<persistence::InventoryEntry> InventoryImpl::listItems() const {
    if (!session_.isActive()) return {};
    return repo_.listForUser(session_.user()->id);
}

void InventoryImpl::addItem(std::int64_t itemId, int quantity) {
    if (!session_.isActive()) return;
    repo_.addItem(session_.user()->id, itemId, quantity);
}

bool InventoryImpl::useItem(std::int64_t itemId, int quantity) {
    if (!session_.isActive()) return false;
    return repo_.removeItem(session_.user()->id, itemId, quantity);
}

} // namespace bitwave::player