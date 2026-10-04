#include "persistence/SqliteTransactionRepository.h"

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

const char* statusToText(TransactionStatus status) {
    return status == TransactionStatus::Succeeded ? "succeeded" : "failed";
}

TransactionStatus statusFromText(const std::string& text) {
    return text == "succeeded" ? TransactionStatus::Succeeded : TransactionStatus::Failed;
}

} // namespace

SqliteTransactionRepository::SqliteTransactionRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS transactions ("
        "  id        INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id   INTEGER NOT NULL REFERENCES users(id),"
        "  item_id   INTEGER NOT NULL REFERENCES store_items(id),"
        "  amount    INTEGER NOT NULL,"
        "  timestamp TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "  status    TEXT    NOT NULL"
        ");");
}

std::int64_t SqliteTransactionRepository::record(std::int64_t userId, std::int64_t itemId,
                                                  long long amount, TransactionStatus status) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO transactions (user_id, item_id, amount, status) VALUES (?, ?, ?, ?);");
    sqlite3_bind_int64(stmt.get(), 1, userId);
    sqlite3_bind_int64(stmt.get(), 2, itemId);
    sqlite3_bind_int64(stmt.get(), 3, amount);
    sqlite3_bind_text(stmt.get(), 4, statusToText(status), -1, SQLITE_STATIC);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not record transaction: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
    return sqlite3_last_insert_rowid(db_.handle());
}

std::vector<TransactionRecord> SqliteTransactionRepository::listForUser(std::int64_t userId) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, user_id, item_id, amount, timestamp, status "
        "FROM transactions WHERE user_id = ? ORDER BY id;");
    sqlite3_bind_int64(stmt.get(), 1, userId);

    std::vector<TransactionRecord> records;
    while (sqlite3_step(stmt.get()) == SQLITE_ROW) {
        TransactionRecord record;
        record.id = sqlite3_column_int64(stmt.get(), 0);
        record.userId = sqlite3_column_int64(stmt.get(), 1);
        record.itemId = sqlite3_column_int64(stmt.get(), 2);
        record.amount = sqlite3_column_int64(stmt.get(), 3);
        record.timestamp = columnText(stmt.get(), 4);
        record.status = statusFromText(columnText(stmt.get(), 5));
        records.push_back(record);
    }
    return records;
}

} // namespace bitwave::persistence