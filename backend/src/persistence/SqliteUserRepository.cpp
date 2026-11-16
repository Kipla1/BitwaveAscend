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

UserRecord readUserRow(sqlite3_stmt* stmt) {
    UserRecord record;
    record.id           = sqlite3_column_int64(stmt, 0);
    record.username     = columnText(stmt, 1);
    record.passwordHash = columnText(stmt, 2);
    record.alias        = columnText(stmt, 3);
    record.email        = columnText(stmt, 4);
    record.googleSub    = columnText(stmt, 5);
    return record;
}

// Upgrades pre-existing local databases created before Google sign-in was
// added (when password_hash was NOT NULL and email/google_sub did not exist).
void migrateLegacyUsersTableIfNeeded(Database& db) {
    bool hasEmail = false;
    bool hasGoogleSub = false;
    bool passwordHashNotNull = false;

    {
        auto stmt = prepare(db.handle(), "PRAGMA table_info(users);");
        while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
            const std::string name = columnText(stmt.get(), 1);
            const int notNull = sqlite3_column_int(stmt.get(), 3);
            if (name == "email") hasEmail = true;
            if (name == "google_sub") hasGoogleSub = true;
            if (name == "password_hash" && notNull != 0) passwordHashNotNull = true;
        }
    }

    if (passwordHashNotNull) {
        db.execute("PRAGMA foreign_keys = OFF;");
        db.beginTransaction();
        try {
            db.execute(
                "CREATE TABLE users_new ("
                "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
                "  username      TEXT    NOT NULL UNIQUE,"
                "  password_hash TEXT,"
                "  alias         TEXT    NOT NULL UNIQUE,"
                "  email         TEXT,"
                "  google_sub    TEXT    UNIQUE,"
                "  created_at    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP"
                ");");
            db.execute(
                "INSERT INTO users_new (id, username, password_hash, alias, created_at) "
                "SELECT id, username, password_hash, alias, created_at FROM users;");
            db.execute("DROP TABLE users;");
            db.execute("ALTER TABLE users_new RENAME TO users;");
            db.commitTransaction();
        } catch (...) {
            db.rollbackTransaction();
            db.execute("PRAGMA foreign_keys = ON;");
            throw;
        }
        db.execute("PRAGMA foreign_keys = ON;");
        return;
    }

    if (!hasEmail) {
        db.execute("ALTER TABLE users ADD COLUMN email TEXT;");
    }
    if (!hasGoogleSub) {
        db.execute("ALTER TABLE users ADD COLUMN google_sub TEXT;");
        db.execute("CREATE UNIQUE INDEX IF NOT EXISTS idx_users_google_sub ON users(google_sub);");
    }
}

} // namespace

SqliteUserRepository::SqliteUserRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS users ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username      TEXT    NOT NULL UNIQUE,"
        "  password_hash TEXT,"
        "  alias         TEXT    NOT NULL UNIQUE,"
        "  email         TEXT,"
        "  google_sub    TEXT    UNIQUE,"
        "  created_at    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");");
    migrateLegacyUsersTableIfNeeded(db_);
}

std::optional<UserRecord> SqliteUserRepository::findByUsername(
    const std::string& username) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, username, password_hash, alias, email, google_sub "
        "FROM users WHERE username = ?;");

    sqlite3_bind_text(stmt.get(), 1, username.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }
    return readUserRow(stmt.get());
}

std::optional<UserRecord> SqliteUserRepository::findByGoogleSub(
    const std::string& googleSub) const {
    if (googleSub.empty()) {
        return std::nullopt;
    }
    auto stmt = prepare(db_.handle(),
        "SELECT id, username, password_hash, alias, email, google_sub "
        "FROM users WHERE google_sub = ?;");

    sqlite3_bind_text(stmt.get(), 1, googleSub.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }
    return readUserRow(stmt.get());
}

std::optional<std::int64_t> SqliteUserRepository::create(
    const std::string& username, const std::string& passwordHash) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO users (username, password_hash, alias) VALUES (?, ?, ?);");

    sqlite3_bind_text(stmt.get(), 1, username.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, passwordHash.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, username.c_str(),     -1, SQLITE_TRANSIENT); // Start with the username as the alias.

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

std::optional<std::int64_t> SqliteUserRepository::createGoogleUser(
    const std::string& username, const std::string& email, const std::string& googleSub) {
    if (googleSub.empty()) {
        return std::nullopt;
    }
    auto stmt = prepare(db_.handle(),
        "INSERT INTO users (username, password_hash, alias, email, google_sub) "
        "VALUES (?, NULL, ?, ?, ?);");

    sqlite3_bind_text(stmt.get(), 1, username.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, username.c_str(),  -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 3, email.c_str(),     -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 4, googleSub.c_str(), -1, SQLITE_TRANSIENT);

    const int rc = sqlite3_step(stmt.get());
    if (rc == SQLITE_CONSTRAINT) {
        return std::nullopt; // username, alias, or google_sub already taken
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
