#pragma once

#include <map>
#include "persistence/InventoryRepository.h"

namespace bitwave::persistence {

class InMemoryInventoryRepository : public InventoryRepository {
public:
    std::vector<InventoryEntry> listForUser(std::int64_t userId) const override {
        std::vector<InventoryEntry> entries;
        for (const auto& [key, quantity] : items_) {
            if (key.first == userId) {
                entries.push_back({key.second, quantity});
            }
        }
        return entries;
    }

    void addItem(std::int64_t userId, std::int64_t itemId, int quantity) override {
        items_[{userId, itemId}] += quantity;
    }

    bool removeItem(std::int64_t userId, std::int64_t itemId, int quantity) override {
        auto it = items_.find({userId, itemId});
        if (it == items_.end() || it->second < quantity) return false;
        it->second -= quantity;
        return true;
    }

private:
    std::map<std::pair<std::int64_t, std::int64_t>, int> items_;
};

} // namespace bitwave::persistence