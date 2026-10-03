#include "persistence/SqliteStoreItemRepository.h"

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

shared::StoreItem readRow(sqlite3_stmt* stmt) {
    shared::StoreItem item;
    item.itemId = sqlite3_column_int64(stmt, 0);
    item.name = columnText(stmt, 1);
    item.price = sqlite3_column_int64(stmt, 2);
    item.description = columnText(stmt, 3);
    return item;
}

} // namespace

SqliteStoreItemRepository::SqliteStoreItemRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS store_items ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name        TEXT    NOT NULL,"
        "  price       INTEGER NOT NULL,"
        "  description TEXT    NOT NULL"
        ");");

    // Seed starter items on first run only. This project has no admin
    // tooling to populate the catalog by hand, and a store with nothing
    // in it isn't testable end-to-end — so a one-time seed, guarded by
    // "table is currently empty," stands in for that tooling.
    auto countStmt = prepare(db_.handle(), "SELECT COUNT(*) FROM store_items;");
    sqlite3_step(countStmt.get());
    const bool isEmpty = sqlite3_column_int(countStmt.get(), 0) == 0;

    if (isEmpty) {
        db_.execute(
            "INSERT INTO store_items (name, price, description) VALUES "
            "('Health Potion', 50, 'Restores health during a run'),"
            "('Speed Boost',  100, 'Temporary movement speed increase'),"
            "('Extra Life',   250, 'One additional life on game over');");
    }
}

std::vector<shared::StoreItem> SqliteStoreItemRepository::listAll() const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, name, price, description FROM store_items ORDER BY id;");

    std::vector<shared::StoreItem> items;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        items.push_back(readRow(stmt.get()));
    }
    return items;
}

std::optional<shared::StoreItem> SqliteStoreItemRepository::findById(std::int64_t itemId) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, name, price, description FROM store_items WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, itemId);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }
    return readRow(stmt.get());
}

} // namespace bitwave::persistence