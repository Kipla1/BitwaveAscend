#pragma once

#include <cstdint>
#include <vector>

namespace bitwave::persistence {

struct InventoryEntry {
    std::int64_t itemId = 0;
    int          quantity = 0;
};

class InventoryRepository {
public:
    virtual ~InventoryRepository() = default;

    virtual std::vector<InventoryEntry> listForUser(std::int64_t userId) const = 0;

    // Adds `quantity` of itemId, creating the row if this is the first one.
    virtual void addItem(std::int64_t userId, std::int64_t itemId, int quantity) = 0;

    // True if removed. False if the player doesn't have enough (or any).
    virtual bool removeItem(std::int64_t userId, std::int64_t itemId, int quantity) = 0;
};

} // namespace bitwave::persistence