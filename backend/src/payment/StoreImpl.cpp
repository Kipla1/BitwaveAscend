#include "payment/StoreImpl.h"

namespace bitwave::payment {

StoreImpl::StoreImpl(persistence::StoreItemRepository& repo) : repo_(repo) {}

std::vector<shared::StoreItem> StoreImpl::listItems() const {
    return repo_.listAll();
}

std::optional<shared::StoreItem> StoreImpl::getItem(std::int64_t itemId) const {
    return repo_.findById(itemId);
}

} // namespace bitwave::payment