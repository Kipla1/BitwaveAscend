// Proves the DBMS connection end to end, against a REAL database file.
// Not part of the game: a developer tool, like paystack_smoke.
//
//   db_smoke                      -> uses backend/data/bitwave.db
//   db_smoke path/to/file.db      -> uses that file
//
// It opens the file, creates every table, writes a row, reads it back, and
// exits non-zero on the first failure. Afterwards you can open the same file
// with the sqlite3 command-line shell to see the data with your own eyes.

#include <ctime>
#include <filesystem>
#include <iostream>
#include <string>

#include "persistence/Database.h"
#include "persistence/SqliteInventoryRepository.h"
#include "persistence/SqlitePaymentOrderRepository.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteStoreItemRepository.h"
#include "persistence/SqliteTransactionRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqliteWalletRepository.h"

using namespace bitwave;

namespace {
bool check(bool ok, const std::string& what) {
    std::cout << (ok ? "  [ OK ] " : "  [FAIL] ") << what << "\n";
    return ok;
}
} // namespace

int main(int argc, char** argv) {
    const std::string path = argc > 1 ? argv[1] : "backend/data/bitwave.db";

    try {
        // The data/ folder is git-ignored and may not exist on a fresh clone.
        const std::filesystem::path file(path);
        if (file.has_parent_path()) {
            std::filesystem::create_directories(file.parent_path());
        }

        std::cout << "Bitwave Ascend - database smoke test\n";
        persistence::Database db(path);
        std::cout << "[1/4] Opened database file: " << path << "\n";

        // Each repository creates its own table in its constructor.
        persistence::SqliteUserRepository users(db);
        persistence::SqlitePlayerProfileRepository progress(db);
        persistence::SqliteWalletRepository wallets(db);
        persistence::SqliteStoreItemRepository store(db);
        persistence::SqliteInventoryRepository inventory(db);
        persistence::SqliteTransactionRepository transactions(db);
        persistence::SqlitePaymentOrderRepository orders(db);
        std::cout << "[2/4] Tables created (or already present)\n";

        std::cout << "[3/4] Write then read back\n";
        const std::string name = "smoke_" + std::to_string(std::time(nullptr));
        const auto id = users.create(name, "not-a-real-hash");
        if (!check(id.has_value(), "inserted user '" + name + "'")) return 1;

        progress.create(*id);
        wallets.create(*id);
        wallets.addFunds(*id, 100);

        const auto found = users.findByUsername(name);
        if (!check(found.has_value() && found->id == *id, "read the user back by username")) return 1;

        const auto balance = wallets.getBalance(*id);
        if (!check(balance.has_value() && *balance == 100, "wallet balance is 100")) return 1;

        const auto prog = progress.find(*id);
        if (!check(prog.has_value() && prog->highestScore == 0, "player progress row exists")) return 1;

        const auto items = store.listAll();
        if (!check(!items.empty(), "store catalog has " + std::to_string(items.size()) + " item(s)")) return 1;

        std::cout << "[4/4] ALL CHECKS PASSED - the database connection works.\n"
                  << "Inspect it yourself:  sqlite3 " << path << " \".tables\"\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[FAIL] " << e.what() << "\n";
        return 1;
    }
}
