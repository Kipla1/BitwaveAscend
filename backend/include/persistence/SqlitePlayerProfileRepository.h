#pragma once

#include "persistence/Database.h"
#include "persistence/PlayerProfileRepository.h"

namespace bitwave::persistence {

class SqlitePlayerProfileRepository : public PlayerProfileRepository {
public:
    explicit SqlitePlayerProfileRepository(Database& db);

    void create(std::int64_t userId) override;
    std::optional<ProgressRecord> find(std::int64_t userId) const override;
    void updateHighestScore(std::int64_t userId, long long score) override;
    void updateLevel(std::int64_t userId, int level) override;

private:
    Database& db_;
};

} // namespace bitwave::persistence