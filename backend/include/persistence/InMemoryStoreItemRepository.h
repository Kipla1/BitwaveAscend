#pragma once

#include <vector>
#include "persistence/StoreItemRepository.h"

namespace bitwave::persistence {

class InMemoryStoreItemRepository : public StoreItemRepository {
public:
    // Tests seed their own items explicitly — no auto-seeding here, unlike
    // the SQLite version, so each test starts from a known, minimal state.
    void addItem(shared::StoreItem item) {
        items_.push_back(std::move(item));
    }

    std::vector<shared::StoreItem> listAll() const override {
        return items_;
    }

    std::optional<shared::StoreItem> findById(std::int64_t itemId) const override {
        for (const auto& item : items_) {
            if (item.itemId == itemId) return item;
        }
        return std::nullopt;
    }

private:
    std::vector<shared::StoreItem> items_;
};

} // namespace bitwave::persistence