#pragma once

#include "persistence/Database.h"
#include "persistence/UserRepository.h"

namespace bitwave::persistence {

class SqliteUserRepository : public UserRepository {
public:
    // `db` must outlive this object. Creates the users table if missing.
    explicit SqliteUserRepository(Database& db);

    std::optional<UserRecord> findByUsername(const std::string& username) const override;

    std::optional<std::int64_t> create(const std::string& username,
                                       const std::string& passwordHash) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence