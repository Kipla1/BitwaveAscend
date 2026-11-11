#pragma once

#include "persistence/Database.h"
#include "persistence/LeaderboardRepository.h"

namespace bitwave::persistence {

class SqliteLeaderboardRepository final : public LeaderboardRepository {
public:
    explicit SqliteLeaderboardRepository(Database& db) : db_(db) {}
    std::vector<shared::LeaderboardEntry> top(std::size_t limit) const override;

private:
    Database& db_;
};

} // namespace bitwave::persistence
