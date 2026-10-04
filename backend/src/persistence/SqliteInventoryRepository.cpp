#include "persistence/SqliteInventoryRepository.h"

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

SqliteInventoryRepository::SqliteInventoryRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS inventory ("
        "  user_id  INTEGER NOT NULL REFERENCES users(id),"
        "  item_id  INTEGER NOT NULL REFERENCES store_items(id),"
        "  quantity INTEGER NOT NULL DEFAULT 1,"
        "  PRIMARY KEY (user_id, item_id)"
        ");");
}

std::vector<InventoryEntry> SqliteInventoryRepository::listForUser(std::int64_t userId) const {
    auto stmt = prepare(db_.handle(),
        "SELECT item_id, quantity FROM inventory WHERE user_id = ? ORDER BY item_id;");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    std::vector<InventoryEntry> entries;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        InventoryEntry entry;
        entry.itemId = sqlite3_column_int64(stmt.get(), 0);
        entry.quantity = sqlite3_column_int(stmt.get(), 1);
        entries.push_back(entry);
    }
    return entries;
}

void SqliteInventoryRepository::addItem(std::int64_t userId, std::int64_t itemId, int quantity) {
    // Upsert: insert a new row at `quantity`, or if (user_id, item_id)
    // already exists (the composite PRIMARY KEY conflict), add to the
    // existing quantity instead — one atomic statement, no race window.
    auto stmt = prepare(db_.handle(),
        "INSERT INTO inventory (user_id, item_id, quantity) VALUES (?, ?, ?) "
        "ON CONFLICT(user_id, item_id) DO UPDATE SET quantity = quantity + excluded.quantity;");
    sqlite3_bind_int64(stmt.get(), 1, userId);
    sqlite3_bind_int64(stmt.get(), 2, itemId);
    sqlite3_bind_int(stmt.get(), 3, quantity);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not add item: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

bool SqliteInventoryRepository::removeItem(std::int64_t userId, std::int64_t itemId, int quantity) {
    // Same constrained-write pattern as Wallet::spend — the quantity check
    // and the decrement happen in one atomic WHERE clause.
    auto stmt = prepare(db_.handle(),
        "UPDATE inventory SET quantity = quantity - ? "
        "WHERE user_id = ? AND item_id = ? AND quantity >= ?;");
    sqlite3_bind_int(stmt.get(), 1, quantity);
    sqlite3_bind_int64(stmt.get(), 2, userId);
    sqlite3_bind_int64(stmt.get(), 3, itemId);
    sqlite3_bind_int(stmt.get(), 4, quantity);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not remove item: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_changes(db_.handle()) > 0;
}

} // namespace bitwave::persistence