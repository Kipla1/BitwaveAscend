#pragma once

#include "persistence/AdminRepository.h"
#include "persistence/Database.h"

namespace bitwave::persistence {

class SqliteAdminRepository : public AdminRepository {
public:
    // The other repositories must be constructed first on the same Database:
    // they are the ones that create the tables this class reads.
    explicit SqliteAdminRepository(Database& db);

    AdminSummary summary() const override;
    std::vector<AdminUserRow> listUsers() const override;
    std::vector<AdminPurchaseRow> listPurchases(int limit) const override;
    std::vector<AdminOrderRow> listOrders(int limit) const override;
    std::int64_t addStoreItem(const std::string& name, long long price,
                              const std::string& description) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence
