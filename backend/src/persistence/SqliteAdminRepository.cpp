#include "persistence/SqliteAdminRepository.h"

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

std::string columnText(sqlite3_stmt* stmt, int column) {
    const unsigned char* text = sqlite3_column_text(stmt, column);
    return text ? reinterpret_cast<const char*>(text) : std::string{};
}

} // namespace

SqliteAdminRepository::SqliteAdminRepository(Database& db) : db_(db) {}

AdminSummary SqliteAdminRepository::summary() const {
    // One round trip: each column is its own small sub-query.
    auto stmt = prepare(db_.handle(),
        "SELECT"
        " (SELECT COUNT(*) FROM users),"
        " (SELECT COALESCE(SUM(balance), 0) FROM wallets),"
        " (SELECT COUNT(*) FROM transactions WHERE status = 'succeeded'),"
        " (SELECT COUNT(*) FROM payment_orders WHERE status = 'succeeded'),"
        " (SELECT COALESCE(SUM(amount_cents), 0) FROM payment_orders WHERE status = 'succeeded');");

    AdminSummary out;
    if (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        out.totalUsers          = sqlite3_column_int64(stmt.get(), 0);
        out.coinsInCirculation  = sqlite3_column_int64(stmt.get(), 1);
        out.successfulPurchases = sqlite3_column_int64(stmt.get(), 2);
        out.successfulTopUps    = sqlite3_column_int64(stmt.get(), 3);
        out.revenueCents        = sqlite3_column_int64(stmt.get(), 4);
    }
    return out;
}

std::vector<AdminUserRow> SqliteAdminRepository::listUsers() const {
    // LEFT JOIN: a user with a missing wallet/progress row still shows up.
    auto stmt = prepare(db_.handle(),
        "SELECT u.id, u.username, u.alias, u.email,"
        "       u.password_hash IS NOT NULL, u.google_sub IS NOT NULL,"
        "       COALESCE(w.balance, 0), COALESCE(p.highest_score, 0),"
        "       COALESCE(p.current_level, 0), u.created_at "
        "FROM users u "
        "LEFT JOIN wallets w ON w.user_id = u.id "
        "LEFT JOIN player_progress p ON p.user_id = u.id "
        "ORDER BY u.id;");

    std::vector<AdminUserRow> rows;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        AdminUserRow row;
        row.id           = sqlite3_column_int64(stmt.get(), 0);
        row.username     = columnText(stmt.get(), 1);
        row.alias        = columnText(stmt.get(), 2);
        row.email        = columnText(stmt.get(), 3);
        row.hasPassword  = sqlite3_column_int(stmt.get(), 4) != 0;
        row.hasGoogle    = sqlite3_column_int(stmt.get(), 5) != 0;
        row.balance      = sqlite3_column_int64(stmt.get(), 6);
        row.highestScore = sqlite3_column_int64(stmt.get(), 7);
        row.currentLevel = sqlite3_column_int(stmt.get(), 8);
        row.createdAt    = columnText(stmt.get(), 9);
        rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<AdminPurchaseRow> SqliteAdminRepository::listPurchases(int limit) const {
    auto stmt = prepare(db_.handle(),
        "SELECT t.id, u.username, s.name, t.amount, t.timestamp, t.status "
        "FROM transactions t "
        "JOIN users u ON u.id = t.user_id "
        "JOIN store_items s ON s.id = t.item_id "
        "ORDER BY t.id DESC LIMIT ?;");
    sqlite3_bind_int(stmt.get(), 1, limit);

    std::vector<AdminPurchaseRow> rows;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        AdminPurchaseRow row;
        row.id        = sqlite3_column_int64(stmt.get(), 0);
        row.username  = columnText(stmt.get(), 1);
        row.itemName  = columnText(stmt.get(), 2);
        row.amount    = sqlite3_column_int64(stmt.get(), 3);
        row.timestamp = columnText(stmt.get(), 4);
        row.status    = columnText(stmt.get(), 5);
        rows.push_back(std::move(row));
    }
    return rows;
}

std::vector<AdminOrderRow> SqliteAdminRepository::listOrders(int limit) const {
    auto stmt = prepare(db_.handle(),
        "SELECT o.id, u.username, o.reference, o.amount_cents, o.currency,"
        "       o.coins, o.status, o.created_at "
        "FROM payment_orders o "
        "JOIN users u ON u.id = o.user_id "
        "ORDER BY o.id DESC LIMIT ?;");
    sqlite3_bind_int(stmt.get(), 1, limit);

    std::vector<AdminOrderRow> rows;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        AdminOrderRow row;
        row.id          = sqlite3_column_int64(stmt.get(), 0);
        row.username    = columnText(stmt.get(), 1);
        row.reference   = columnText(stmt.get(), 2);
        row.amountCents = sqlite3_column_int64(stmt.get(), 3);
        row.currency    = columnText(stmt.get(), 4);
        row.coins       = sqlite3_column_int64(stmt.get(), 5);
        row.status      = columnText(stmt.get(), 6);
        row.createdAt   = columnText(stmt.get(), 7);
        rows.push_back(std::move(row));
    }
    return rows;
}

std::int64_t SqliteAdminRepository::addStoreItem(const std::string& name, long long price,
                                                 const std::string& description) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO store_items (name, price, description) VALUES (?, ?, ?);");
    sqlite3_bind_text(stmt.get(), 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 2, price);
    sqlite3_bind_text(stmt.get(), 3, description.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not add store item: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_last_insert_rowid(db_.handle());
}

} // namespace bitwave::persistence
