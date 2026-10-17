#include "persistence/SqlitePaymentOrderRepository.h"

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

OrderStatus statusFromText(const std::string& text) {
    if (text == "succeeded") return OrderStatus::Succeeded;
    if (text == "failed")    return OrderStatus::Failed;
    return OrderStatus::Pending;
}

// The WHERE status = 'pending' is the whole trick: the check and the
// change are one atomic statement, same idea as Wallet::spend.
bool moveFromPending(Database& db, const std::string& reference, const char* newStatus) {
    auto stmt = prepare(db.handle(),
        "UPDATE payment_orders SET status = ? WHERE reference = ? AND status = 'pending';");
    sqlite3_bind_text(stmt.get(), 1, newStatus, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt.get(), 2, reference.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not update payment order: ") +
                                 sqlite3_errmsg(db.handle()));
    }
    return sqlite3_changes(db.handle()) > 0;
}

} // namespace

SqlitePaymentOrderRepository::SqlitePaymentOrderRepository(Database& db) : db_(db) {
    db_.execute(
        "CREATE TABLE IF NOT EXISTS payment_orders ("
        "  id           INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  user_id      INTEGER NOT NULL REFERENCES users(id),"
        "  reference    TEXT    NOT NULL UNIQUE,"
        "  amount_cents INTEGER NOT NULL,"
        "  currency     TEXT    NOT NULL,"
        "  coins        INTEGER NOT NULL,"
        "  status       TEXT    NOT NULL DEFAULT 'pending',"
        "  created_at   TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");");
}

void SqlitePaymentOrderRepository::create(std::int64_t userId, const std::string& reference,
                                          long long amountCents, const std::string& currency,
                                          long long coins) {
    auto stmt = prepare(db_.handle(),
        "INSERT INTO payment_orders (user_id, reference, amount_cents, currency, coins) "
        "VALUES (?, ?, ?, ?, ?);");
    sqlite3_bind_int64(stmt.get(), 1, userId);
    sqlite3_bind_text(stmt.get(), 2, reference.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 3, amountCents);
    sqlite3_bind_text(stmt.get(), 4, currency.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt.get(), 5, coins);

    if (sqlite3_step(stmt.get()) != SQLITE_DONE) {
        throw std::runtime_error(std::string("Could not create payment order: ") +
                                 sqlite3_errmsg(db_.handle()));
    }
}

std::optional<PaymentOrder> SqlitePaymentOrderRepository::findByReference(
    const std::string& reference) const {
    auto stmt = prepare(db_.handle(),
        "SELECT id, user_id, reference, amount_cents, currency, coins, status "
        "FROM payment_orders WHERE reference = ?;");
    sqlite3_bind_text(stmt.get(), 1, reference.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt.get()) != SQLITE_ROW) {
        return std::nullopt;
    }

    PaymentOrder order;
    order.id = sqlite3_column_int64(stmt.get(), 0);
    order.userId = sqlite3_column_int64(stmt.get(), 1);
    order.reference = columnText(stmt.get(), 2);
    order.amountCents = sqlite3_column_int64(stmt.get(), 3);
    order.currency = columnText(stmt.get(), 4);
    order.coins = sqlite3_column_int64(stmt.get(), 5);
    order.status = statusFromText(columnText(stmt.get(), 6));
    return order;
}

bool SqlitePaymentOrderRepository::markSucceeded(const std::string& reference) {
    return moveFromPending(db_, reference, "succeeded");
}

bool SqlitePaymentOrderRepository::markFailed(const std::string& reference) {
    return moveFromPending(db_, reference, "failed");
}

} // namespace bitwave::persistence