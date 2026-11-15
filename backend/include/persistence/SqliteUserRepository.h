#pragma once

#include "persistence/Database.h"
#include "persistence/UserRepository.h"

namespace bitwave::persistence {

class SqliteUserRepository : public UserRepository {
public:
    // `db` must outlive this object. Creates or migrates the users table.
    explicit SqliteUserRepository(Database& db);

    std::optional<UserRecord> findByUsername(const std::string& username) const override;

    std::optional<UserRecord> findByGoogleSub(const std::string& googleSub) const override;

    std::optional<std::int64_t> create(const std::string& username,
                                       const std::string& passwordHash) override;

    std::optional<std::int64_t> createGoogleUser(const std::string& username,
                                                 const std::string& email,
                                                 const std::string& googleSub) override;

    bool isAliasAvailable(const std::string& alias) const override;

    bool updateAlias(std::int64_t userId, const std::string& alias) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence