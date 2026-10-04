#include "persistence/SqliteUserRepository.h"

#include <memory>
#include <sqlite3.h>
#include <stdexcept>

namespace bitwave::persistence {

namespace {

// Guarantees sqlite3_finalize runs on every exit path, including early returns.
using Statement = std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)>;

Statement prepare(sqlite3* db, const char* sql) {
    sqlite3_stmt* raw = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &raw, nullptr) != SQLITE_OK) {
        throw std::runtime_error(std::string("Could not prepare statement: ") +
                                 sqlite3_errmsg(db));
    }
    return Statement(raw, &sqlite3_finalize);
}

std::string columnText(sqlite3_stmt* stmt, int column) {
    const unsigned char* text = sqlite3_column_text(stmt, column);
    return text ? reinterpret_cast<const char*>(text) : std::string{};
}

} // namespace

SqliteUserRepository::SqliteUserRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username      TEXT    NOT NULL UNIQUE,"
        "  password_hash TEXT    NOT NULL,"
        "  alias         TEXT    NOT NULL UNIQUE,"
        "  created_at    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");");
}

std::optional<UserRecord> SqliteUserRepository::findByUsername(
    const std::string& username) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, username, password_hash, alias FROM users WHERE username = ?;");

    sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }

    UserRecord record;
    record.id           = sqlite3_column_int64(stmt.get(), 0);
    record.username     = columnText(stmt.get(), 1);
    record.passwordHash = columnText(stmt.get(), 2);
    record.alias        = columnText(stmt.get(), 3);
    return record;
}

std::optional<std::int64_t> SqliteUserRepository::create(
    const std::string& username, const std::string& passwordHash) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO users (username, password_hash, alias) VALUES (?, ?, ?);");

    sqlite3_bind_text(stmt.get(), 1, username.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, username.c_str(),     -1, SQLITE_TRANSIENT); // alias placeholder (Day 4)

    const int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_CONSTRAINT) {
        return std::nullopt; // username already taken
    }
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(std::string("Insert failed: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_last_insert_rowid(db_.handle());
}

bool SqliteUserRepository::isAliasAvailable(const std::string& alias) const {
    auto stmt = prepare(db_.handle(), "SELECT 1 FROM users WHERE alias = ?;");
    sqlite3_bind_text(stmt.get(), 1, alias.c_str(), -1, SQLITE_TRANSIENT);
    return sqlite3_step(stmt.get()) != SQLITE_ROW; // no row found = available
}

bool SqliteUserRepository::updateAlias(std::int64_t userId, const std::string& alias) {
    auto stmt = prepare(db_.handle(), "UPDATE users SET alias = ? WHERE id = ?;");
    sqlite3_bind_text(stmt.get(), 1, alias.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 2, userId);

    const int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_CONSTRAINT) {
        return false; // alias already taken — the UNIQUE constraint caught it
    }
    if (rc != SQLITE_DONE) {
        throw std::runtime_error(std::string("Update failed: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_changes(db_.handle()) > 0; // false if userId didn't exist
}

} // namespace bitwave::persistence