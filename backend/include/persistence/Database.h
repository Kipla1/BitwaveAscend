#pragma once

#include <string>

struct sqlite3; // forward declaration: keeps <sqlite3.h> out of every includer

namespace bitwave::persistence {

// RAII owner of one SQLite connection. Opens in the constructor, closes in
// the destructor. Pass ":memory:" as the path for a throwaway test database.
class Database {
public:
    explicit Database(const std::string& path); // throws std::runtime_error
    ~Database();

    Database(const Database&) = delete;            // one connection, one owner
    Database& operator=(const Database&) = delete;

    sqlite3* handle() const { return db_; }

    // For statements with no user input (schema setup). Throws on failure.
    void execute(const std::string& sql);

private:
    sqlite3* db_ = nullptr;
};

} // namespace bitwave::persistence