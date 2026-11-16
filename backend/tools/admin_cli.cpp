// Bitwave Ascend - admin panel (terminal edition).
//
// Talks to the database ONLY through repository interfaces, like the rest of
// the backend. Read-only views plus one management action: adding items to
// the store catalog (the project has no other way to do that).
//
//   BITWAVE_ADMIN_PASSWORD=secret admin_cli [path/to/bitwave.db]

#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "persistence/Database.h"
#include "persistence/SqliteAdminRepository.h"
#include "persistence/SqliteInventoryRepository.h"
#include "persistence/SqlitePaymentOrderRepository.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteStoreItemRepository.h"
#include "persistence/SqliteTransactionRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqliteWalletRepository.h"

using namespace bitwave;

namespace {

bool readLine(const std::string& prompt, std::string& out) {
    std::cout << prompt;
    return static_cast<bool>(std::getline(std::cin, out));
}

// Parses the WHOLE line as a number; "12abc" is rejected.
bool readNumber(const std::string& prompt, long long& out) {
    std::string line;
    if (!readLine(prompt, line)) return false;
    try {
        std::size_t used = 0;
        out = std::stoll(line, &used);
        return used == line.size();
    } catch (...) {
        return false;
    }
}

std::string kes(long long cents) {
    std::ostringstream out;
    out << cents / 100 << "." << std::setw(2) << std::setfill('0') << cents % 100;
    return out.str();
}

void showSummary(const persistence::AdminRepository& admin) {
    const auto s = admin.summary();
    std::cout << "\n--- Dashboard ---\n"
              << "Registered players     : " << s.totalUsers << "\n"
              << "Coins in circulation   : " << s.coinsInCirculation << "\n"
              << "Store purchases        : " << s.successfulPurchases << "\n"
              << "Successful top-ups     : " << s.successfulTopUps << "\n"
              << "Revenue (KES)          : " << kes(s.revenueCents) << "\n";
}

void showUsers(const persistence::AdminRepository& admin) {
    const auto rows = admin.listUsers();
    std::cout << "\n--- Players (" << rows.size() << ") ---\n"
              << std::left << std::setw(5) << "ID" << std::setw(22) << "Username"
              << std::setw(22) << "Alias" << std::setw(10) << "Login"
              << std::setw(10) << "Coins" << std::setw(10) << "Best" << "Level\n";
    for (const auto& u : rows) {
        const std::string login = u.hasGoogle ? (u.hasPassword ? "both" : "google") : "password";
        std::cout << std::left << std::setw(5) << u.id << std::setw(22) << u.username
                  << std::setw(22) << u.alias << std::setw(10) << login
                  << std::setw(10) << u.balance << std::setw(10) << u.highestScore
                  << u.currentLevel << "\n";
    }
}

void showPurchases(const persistence::AdminRepository& admin) {
    const auto rows = admin.listPurchases(20);
    std::cout << "\n--- Latest store purchases (max 20) ---\n"
              << std::left << std::setw(5) << "ID" << std::setw(22) << "Player"
              << std::setw(18) << "Item" << std::setw(8) << "Coins"
              << std::setw(22) << "When" << "Status\n";
    for (const auto& p : rows) {
        std::cout << std::left << std::setw(5) << p.id << std::setw(22) << p.username
                  << std::setw(18) << p.itemName << std::setw(8) << p.amount
                  << std::setw(22) << p.timestamp << p.status << "\n";
    }
}

void showOrders(const persistence::AdminRepository& admin) {
    const auto rows = admin.listOrders(20);
    std::cout << "\n--- Latest top-up orders (max 20) ---\n"
              << std::left << std::setw(5) << "ID" << std::setw(22) << "Player"
              << std::setw(14) << "Paid" << std::setw(8) << "Coins"
              << std::setw(11) << "Status" << "Reference\n";
    for (const auto& o : rows) {
        std::cout << std::left << std::setw(5) << o.id << std::setw(22) << o.username
                  << std::setw(14) << (o.currency + " " + kes(o.amountCents))
                  << std::setw(8) << o.coins << std::setw(11) << o.status
                  << o.reference << "\n";
    }
}

void showStore(const persistence::StoreItemRepository& store) {
    const auto items = store.listAll();
    std::cout << "\n--- Store catalog ---\n"
              << std::left << std::setw(5) << "ID" << std::setw(18) << "Name"
              << std::setw(8) << "Price" << "Description\n";
    for (const auto& item : items) {
        std::cout << std::left << std::setw(5) << item.itemId << std::setw(18) << item.name
                  << std::setw(8) << item.price << item.description << "\n";
    }
}

void addStoreItem(persistence::AdminRepository& admin) {
    std::string name, description;
    long long price = 0;

    if (!readLine("Item name: ", name) || name.empty() || name.size() > 40) {
        std::cout << "Name must be 1-40 characters.\n";
        return;
    }
    if (!readNumber("Price in coins: ", price) || price <= 0) {
        std::cout << "Price must be a whole number above 0.\n";
        return;
    }
    if (!readLine("Description: ", description) || description.empty()) {
        std::cout << "Description is required.\n";
        return;
    }

    const auto id = admin.addStoreItem(name, price, description);
    std::cout << "Added '" << name << "' to the store (id " << id << ").\n";
}

} // namespace

int main(int argc, char** argv) {
    // Temporary gate: the users table has no admin role yet.
    const char* expected = std::getenv("BITWAVE_ADMIN_PASSWORD");
    if (!expected || std::string(expected).empty()) {
        std::cerr << "Set BITWAVE_ADMIN_PASSWORD before starting the admin panel.\n";
        return 1;
    }
    std::string attempt;
    if (!readLine("Admin password: ", attempt) || attempt != expected) {
        std::cerr << "Access denied.\n";
        return 1;
    }

    const std::string path = argc > 1 ? argv[1] : "backend/data/bitwave.db";
    try {
        const std::filesystem::path file(path);
        if (file.has_parent_path()) {
            std::filesystem::create_directories(file.parent_path());
        }
        persistence::Database db(path);

        // Same construction order as the game: these create the tables.
        persistence::SqliteUserRepository users(db);
        persistence::SqlitePlayerProfileRepository progress(db);
        persistence::SqliteWalletRepository wallets(db);
        persistence::SqliteStoreItemRepository store(db);
        persistence::SqliteInventoryRepository inventory(db);
        persistence::SqliteTransactionRepository transactions(db);
        persistence::SqlitePaymentOrderRepository orders(db);
        persistence::SqliteAdminRepository sqliteAdmin(db);

        // The menu only knows the abstract types.
        persistence::AdminRepository& admin = sqliteAdmin;
        persistence::StoreItemRepository& catalog = store;

        std::cout << "\nConnected to " << path << "\n";
        while (true) {
            std::cout << "\n=== Bitwave Ascend Admin ===\n"
                         "1) Dashboard   2) Players   3) Purchases   4) Top-ups\n"
                         "5) Store items 6) Add store item          0) Exit\n";
            long long choice = -1;
            if (!readNumber("> ", choice)) {
                if (std::cin.eof()) break;
                std::cout << "Please enter a number from the menu.\n";
                continue;
            }
            switch (choice) {
                case 1: showSummary(admin); break;
                case 2: showUsers(admin); break;
                case 3: showPurchases(admin); break;
                case 4: showOrders(admin); break;
                case 5: showStore(catalog); break;
                case 6: addStoreItem(admin); break;
                case 0: std::cout << "Goodbye.\n"; return 0;
                default: std::cout << "Unknown option.\n";
            }
        }
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Admin panel error: " << e.what() << "\n";
        return 1;
    }
}
