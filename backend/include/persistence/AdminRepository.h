#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace bitwave::persistence {

// One row of the admin "users" table. Deliberately has NO password hash:
// not even the admin tool needs to see it, only whether one exists.
struct AdminUserRow {
    std::int64_t id = 0;
    std::string  username;
    std::string  alias;
    std::string  email;
    bool         hasPassword = false;   // false for Google-only accounts
    bool         hasGoogle = false;
    long long    balance = 0;
    long long    highestScore = 0;
    int          currentLevel = 0;
    std::string  createdAt;
};

// A store purchase (coins spent on an item), with names instead of raw ids.
struct AdminPurchaseRow {
    std::int64_t id = 0;
    std::string  username;
    std::string  itemName;
    long long    amount = 0;
    std::string  timestamp;
    std::string  status;
};

// A real-money top-up order (coins bought with KES via Paystack).
struct AdminOrderRow {
    std::int64_t id = 0;
    std::string  username;
    std::string  reference;
    long long    amountCents = 0;
    std::string  currency;
    long long    coins = 0;
    std::string  status;
    std::string  createdAt;
};

struct AdminSummary {
    long long totalUsers = 0;
    long long coinsInCirculation = 0;     // sum of every wallet balance
    long long successfulPurchases = 0;
    long long successfulTopUps = 0;
    long long revenueCents = 0;           // sum of succeeded top-ups
};

// Read-mostly view of the whole database for the admin panel.
// Like every repository, it is an abstraction: the admin tool only sees
// this interface, never SQLite.
class AdminRepository {
public:
    virtual ~AdminRepository() = default;

    virtual AdminSummary summary() const = 0;
    virtual std::vector<AdminUserRow> listUsers() const = 0;
    virtual std::vector<AdminPurchaseRow> listPurchases(int limit) const = 0;
    virtual std::vector<AdminOrderRow> listOrders(int limit) const = 0;

    // Adds an item to the store catalog. Returns the new item's id.
    virtual std::int64_t addStoreItem(const std::string& name, long long price,
                                      const std::string& description) = 0;
};

} // namespace bitwave::persistence
