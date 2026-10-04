#include "persistence/Database.h"

#include <sqlite3.h>
#include <stdexcept>

namespace bitwave::persistence {

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        const std::string message = db_ ? sqlite3_errmsg(db_) : "out of memory";
        sqlite3_close(db_); // safe even if open failed partway
        db_ = nullptr;
        throw std::runtime_error("Could not open database: " + message);
    }
    execute("PRAGMA foreign_keys = ON;");
}

void Database::beginTransaction() {
    execute("BEGIN TRANSACTION;");
}

void Database::commitTransaction() {
    execute("COMMIT;");
}

void Database::rollbackTransaction() {
    execute("ROLLBACK;");
}

Database::~Database() {
    sqlite3_close(db_);
}

void Database::execute(const std::string& sql) {
    char* error = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &error) != SQLITE_OK) {
        const std::string message = error ? error : "unknown error";
        sqlite3_free(error);
        throw std::runtime_error("SQL error: " + message);
    }
}

} // namespace bitwave::persistence