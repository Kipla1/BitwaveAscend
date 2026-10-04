#include <catch2/catch_test_macros.hpp>

#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "payment/PaymentServiceImpl.h"
#include "payment/StoreImpl.h"
#include "payment/WalletImpl.h"
#include "persistence/Database.h"
#include "persistence/SqliteInventoryRepository.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteStoreItemRepository.h"
#include "persistence/SqliteTransactionRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqliteWalletRepository.h"
#include "player/Inventory.h"

using namespace bitwave;

namespace {

struct Fixture {
    persistence::Database db{":memory:"};
    persistence::SqliteUserRepository users{db};
    persistence::SqlitePlayerProfileRepository progress{db};
    persistence::SqliteWalletRepository walletRepo{db};
    persistence::SqliteStoreItemRepository storeRepo{db}; // seeds item id 1 @ price 50
    persistence::SqliteInventoryRepository inventoryRepo{db};
    persistence::SqliteTransactionRepository transactions{db};
    auth::PasswordHasher hasher;
    auth::Session session;
    auth::AuthServiceImpl authService{users, progress, walletRepo, hasher, session};
    payment::WalletImpl wallet{walletRepo, session};
    payment::StoreImpl store{storeRepo};
    player::InventoryImpl inventory{inventoryRepo, session};
    payment::PaymentServiceImpl payments{db, store, wallet, inventory, transactions, session};
};

} // namespace

TEST_CASE("PaymentService completes a purchase atomically", "[payment]") {
    Fixture f;
    f.authService.registerUser("oscar_k", "longenough1");
    f.authService.login("oscar_k", "longenough1");
    f.wallet.addFunds(100);

    auto result = f.payments.purchaseItem(1); // seeded item, price 50
    REQUIRE(result.success);
    REQUIRE(result.amount == 50);
    REQUIRE(f.wallet.getBalance() == 50);
    REQUIRE(f.inventory.listItems().size() == 1);
    REQUIRE(f.transactions.listForUser(1).size() == 1);
}

TEST_CASE("PaymentService rolls back cleanly on insufficient funds", "[payment]") {
    Fixture f;
    f.authService.registerUser("oscar_k", "longenough1");
    f.authService.login("oscar_k", "longenough1");
    f.wallet.addFunds(10); // item costs 50

    auto result = f.payments.purchaseItem(1);
    REQUIRE_FALSE(result.success);

    // The real point of this test: nothing partial was left behind.
    REQUIRE(f.wallet.getBalance() == 10);
    REQUIRE(f.inventory.listItems().empty());
    REQUIRE(f.transactions.listForUser(1).empty());
}

TEST_CASE("PaymentService rejects a nonexistent item before touching the wallet", "[payment]") {
    Fixture f;
    f.authService.registerUser("oscar_k", "longenough1");
    f.authService.login("oscar_k", "longenough1");
    f.wallet.addFunds(100);

    auto result = f.payments.purchaseItem(9999);
    REQUIRE_FALSE(result.success);
    REQUIRE(f.wallet.getBalance() == 100); // untouched
}

TEST_CASE("PaymentService requires an active session", "[payment]") {
    Fixture f;
    auto result = f.payments.purchaseItem(1);
    REQUIRE_FALSE(result.success);
}