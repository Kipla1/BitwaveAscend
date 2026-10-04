#pragma once

#include "persistence/Database.h"
#include "persistence/StoreItemRepository.h"

namespace bitwave::persistence {

class SqliteStoreItemRepository : public StoreItemRepository {
public:
    // Creates the table and, if empty, seeds it with starter items — see
    // the .cpp for why seeding lives here instead of needing a separate
    // admin tool this project has no time to build.
    explicit SqliteStoreItemRepository(Database& db);

    std::vector<shared::StoreItem> listAll() const override;
    std::optional<shared::StoreItem> findById(std::int64_t itemId) const override;

private:
    Database& db_;
};

} // namespace bitwave::persistence