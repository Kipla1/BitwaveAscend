#include "persistence/SqliteWalletRepository.h"

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

SqliteWalletRepository::SqliteWalletRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS wallets ("
        "  user_id INTEGER PRIMARY KEY REFERENCES users(id),"
        "  balance INTEGER NOT NULL DEFAULT 0"
        ");");
}

void SqliteWalletRepository::create(std::int64_t userId) {
    auto stmt = prepare(db_.handle(), "INSERT INTO wallets (user_id) VALUES (?);");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not create wallet: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

std::optional<long long> SqliteWalletRepository::getBalance(std::int64_t userId) const {
    auto stmt = prepare(db_.handle(), "SELECT balance FROM wallets WHERE user_id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }
    return sqlite3_column_int64(stmt.get(), 0);
}

bool SqliteWalletRepository::spend(std::int64_t userId, long long amount) {
    // Check (balance >= amount) and act (subtract) as one atomic write —
    // see Wallet.h / the design discussion for why this isn't a separate
    // getBalance() check followed by a write.
    auto stmt = prepare(db_.handle(),
        "UPDATE wallets SET balance = balance - ? "
        "WHERE user_id = ? AND balance >= ?;");
    sqlite3_bind_int64(stmt.get(), 1, amount);
    sqlite3_bind_int64(stmt.get(), 2, userId);
    sqlite3_bind_int64(stmt.get(), 3, amount);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Spend failed: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_changes(db_.handle()) > 0; // false = insufficient funds (or bad userId)
}

void SqliteWalletRepository::addFunds(std::int64_t userId, long long amount) {
    auto stmt = prepare(db_.handle(),
        "UPDATE wallets SET balance = balance + ? WHERE user_id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, amount);
    sqlite3_bind_int64(stmt.get(), 2, userId);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Add funds failed: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

} // namespace bitwave::persistence