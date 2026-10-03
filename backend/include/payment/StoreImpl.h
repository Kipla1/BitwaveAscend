#pragma once

#include "payment/Store.h"
#include "persistence/StoreItemRepository.h"

namespace bitwave::payment {

class StoreImpl : public Store {
public:
    explicit StoreImpl(persistence::StoreItemRepository& repo);

    std::vector<shared::StoreItem> listItems() const override;
    std::optional<shared::StoreItem> getItem(std::int64_t itemId) const override;

private:
    persistence::StoreItemRepository& repo_;
};

} // namespace bitwave::payment