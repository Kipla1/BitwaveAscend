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
    item.kind = columnText(stmt, 4) == "cosmetic"
        ? shared::StoreItemKind::Cosmetic : shared::StoreItemKind::PowerUp;
    item.resourceKey = columnText(stmt, 5);
    return item;
}

bool hasColumn(sqlite3* db, const char* name) {
    auto stmt = prepare(db, "PRAGMA table_info(store_items);");
    while (sqlite3_step(stmt.get()) == SQLITE_ROW)
        if (columnText(stmt.get(), 1) == name) return true;
    return false;
}

} // namespace

SqliteStoreItemRepository::SqliteStoreItemRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS store_items ("
        "  id          INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  name        TEXT    NOT NULL,"
        "  price       INTEGER NOT NULL,"
        "  description TEXT    NOT NULL,"
        "  kind        TEXT    NOT NULL DEFAULT 'power_up',"
        "  resource_key TEXT   NOT NULL DEFAULT ''"
        ");");

    // Add the cosmetic columns to saves made by the earlier schema.
    if (!hasColumn(db_.handle(), "kind"))
        db_.execute("ALTER TABLE store_items ADD COLUMN kind TEXT NOT NULL DEFAULT 'power_up';");
    if (!hasColumn(db_.handle(), "resource_key"))
        db_.execute("ALTER TABLE store_items ADD COLUMN resource_key TEXT NOT NULL DEFAULT '';" );

    // Seed starter items on first run only. This project has no admin
    // tooling to populate the catalog by hand, and a store with nothing
    // in it isn't testable end-to-end — so a one-time seed, guarded by
    // "table is currently empty," stands in for that tooling.
    auto countStmt = prepare(db_.handle(), "SELECT COUNT(*) FROM store_items;");
    sqlite3_step(countStmt.get());
    const bool isEmpty = sqlite3_column_int(countStmt.get(), 0) == 0;

    if (isEmpty) {
        db_.execute(
        "INSERT INTO store_items (name, price, description, kind, resource_key) VALUES "
        "('Health Potion', 50, 'Restores health during a run', 'power_up', ''),"
        "('Speed Boost',  100, 'Temporary movement speed increase', 'power_up', ''),"
        "('Extra Life',   250, 'One additional life on game over', 'power_up', '');");
    }

    // Paid skins are optional; the built-in skin needs no store item.
    db_.execute(
        "INSERT INTO store_items (name, price, description, kind, resource_key) "
        "SELECT 'Neon Skin', 300, 'A bright alternate look', 'cosmetic', 'skin.neon' "
        "WHERE NOT EXISTS (SELECT 1 FROM store_items WHERE resource_key='skin.neon');"
        "INSERT INTO store_items (name, price, description, kind, resource_key) "
        "SELECT 'Cloud Skin', 500, 'A soft cloud themed look', 'cosmetic', 'skin.cloud' "
        "WHERE NOT EXISTS (SELECT 1 FROM store_items WHERE resource_key='skin.cloud');");
}

std::vector<shared::StoreItem> SqliteStoreItemRepository::listAll() const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, name, price, description, kind, resource_key FROM store_items ORDER BY id;");

    std::vector<shared::StoreItem> items;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        items.push_back(readRow(stmt.get()));
    }
    return items;
}

std::optional<shared::StoreItem> SqliteStoreItemRepository::findById(std::int64_t itemId) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, name, price, description, kind, resource_key FROM store_items WHERE id = ?;");
    sqlite3_bind_int64(stmt.get(), 1, itemId);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }
    return readRow(stmt.get());
}

} // namespace bitwave::persistence
