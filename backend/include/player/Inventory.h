#pragma once

#include <vector>
#include "auth/Session.h"
#include "persistence/InventoryRepository.h"

namespace bitwave::player {

class Inventory {
public:
    virtual ~Inventory() = default;

    // Items for the currently logged-in player. Empty if nobody is logged in.
    virtual std::vector<persistence::InventoryEntry> listItems() const = 0;

    virtual void addItem(std::int64_t itemId, int quantity) = 0;

    virtual bool useItem(std::int64_t itemId, int quantity) = 0;
};

class InventoryImpl : public Inventory {
public:
    InventoryImpl(persistence::InventoryRepository& repo, const auth::Session& session);

    std::vector<persistence::InventoryEntry> listItems() const override;
    void addItem(std::int64_t itemId, int quantity) override;
    bool useItem(std::int64_t itemId, int quantity) override;

private:
    persistence::InventoryRepository& repo_;
    const auth::Session& session_;
};

} // namespace bitwave::player