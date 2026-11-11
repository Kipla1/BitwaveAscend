#include "persistence/SqliteLeaderboardRepository.h"

#include <algorithm>
#include <memory>
#include <sqlite3.h>
#include <stdexcept>

namespace bitwave::persistence {

std::vector<shared::LeaderboardEntry> SqliteLeaderboardRepository::top(
    std::size_t limit) const {
    std::vector<shared::LeaderboardEntry> rows;
    if (limit == 0) return rows;

    sqlite3_stmt* raw = nullptr;
    const char* sql =
        "SELECT u.alias, p.highest_score FROM player_progress p "
        "JOIN users u ON u.id = p.user_id WHERE p.highest_score > 0 "
        "ORDER BY p.highest_score DESC, u.id ASC LIMIT ?;";
    if (sqlite3_prepare_v2(db_.handle(), sql, -1, &raw, nullptr) != SQLITE_OK)
        throw std::runtime_error(sqlite3_errmsg(db_.handle()));
    std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> stmt(raw, sqlite3_finalize);
    sqlite3_bind_int64(stmt.get(), 1, static_cast<sqlite3_int64>(std::min<std::size_t>(limit, 100)));

    int rc;
    while ((rc = sqlite3_step(stmt.get())) == SQLITE_ROW) {
        const auto* alias = sqlite3_column_text(stmt.get(), 0);
        rows.push_back({static_cast<int>(rows.size() + 1),
                        alias ? reinterpret_cast<const char*>(alias) : "",
                        sqlite3_column_int64(stmt.get(), 1)});
    }
    if (rc != SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(db_.handle()));
    return rows;
}

} // namespace bitwave::persistence
