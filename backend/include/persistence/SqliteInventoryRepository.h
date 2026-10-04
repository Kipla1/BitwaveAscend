#pragma once

#include "persistence/Database.h"
#include "persistence/InventoryRepository.h"

namespace bitwave::persistence {

class SqliteInventoryRepository : public InventoryRepository {
public:
    explicit SqliteInventoryRepository(Database& db);

    std::vector<InventoryEntry> listForUser(std::int64_t userId) const override;
    void addItem(std::int64_t userId, std::int64_t itemId, int quantity) override;
    bool removeItem(std::int64_t userId, std::int64_t itemId, int quantity) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence