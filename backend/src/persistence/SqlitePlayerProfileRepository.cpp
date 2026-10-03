#include "persistence/SqlitePlayerProfileRepository.h"

#include <memory>
#include <sqlite3.h>
#include <stdexcept>

namespace bitwave::persistence {

namespace {

using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

Statement prepare(sqlite3* db, const char* sql) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &raw, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Could not prepare statement: ") +
                                 sqlite3_errmsg(db));
    }
    return Statement(raw, &sqlite3_finalize);
}

} // namespace

SqlitePlayerProfileRepository::SqlitePlayerProfileRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS player_progress ("
        "  user_id        INTEGER PRIMARY KEY REFERENCES users(id),"
        "  highest_score  INTEGER NOT NULL DEFAULT 0,"
        "  current_level  INTEGER NOT NULL DEFAULT 0"
        ");");
}

void SqlitePlayerProfileRepository::create(std::int64_t userId) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO player_progress (user_id) VALUES (?);");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not create player progress: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

std::optional<ProgressRecord> SqlitePlayerProfileRepository::find(std::int64_t userId) const {
    auto stmt = prepare(db_.handle(),
        "SELECT highest_score, current_level FROM player_progress WHERE user_id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }

    ProgressRecord record;
    record.highestScore = sqlite3_column_int64(stmt.get(), 0);
    record.currentLevel = sqlite3_column_int(stmt.get(), 1);
    return record;
}

void SqlitePlayerProfileRepository::updateHighestScore(std::int64_t userId, long long score) {
    // Only writes if `score` actually beats the stored value — avoids a
    // read-then-compare-then-write race, same reasoning as alias uniqueness.
    auto stmt = prepare(db_.handle(),
        "UPDATE player_progress SET highest_score = ? "
        "WHERE user_id = ? AND ? > highest_score;");
    sqlite3_bind_int64(stmt.get(), 1, score);
    sqlite3_bind_int64(stmt.get(), 2, userId);
    sqlite3_bind_int64(stmt.get(), 3, score);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not update highest score: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

void SqlitePlayerProfileRepository::updateLevel(std::int64_t userId, int level) {
    auto stmt = prepare(db_.handle(),
        "UPDATE player_progress SET current_level = ? WHERE user_id = ?;");
    sqlite3_bind_int(stmt.get(), 1, level);
    sqlite3_bind_int64(stmt.get(), 2, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not update level: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

} // namespace bitwave::persistence