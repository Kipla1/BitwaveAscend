#include <catch2/catch_test_macros.hpp>

#include "payment/FakePaymentGateway.h"
#include "payment/TopUpServiceImpl.h"
#include "payment/WalletImpl.h"
#include "persistence/Database.h"
#include "persistence/SqlitePaymentOrderRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqliteWalletRepository.h"

using namespace bitwave;

namespace {

struct Fixture {
    persistence::Database db{":memory:"};
    persistence::SqliteUserRepository users{db};
    persistence::SqliteWalletRepository walletRepo{db};
    persistence::SqlitePaymentOrderRepository orders{db};
    auth::Session session;
    payment::WalletImpl wallet{walletRepo, session};
    payment::FakePaymentGateway gateway;
    payment::TopUpServiceImpl topUp{db, gateway, orders, wallet, session};

    // Skips AuthService (and its slow password hashing): creates the rows
    // registration would, then starts a session.
    std::int64_t signIn(const std::string& name) {
        const auto id = *users.create(name, "hash");
        walletRepo.create(id);
        session.start({id, name, name});
        return id;
    }
};

} // namespace

TEST_CASE("startTopUp requires a logged-in player", "[payment]") {
    Fixture f;
    auto result = f.topUp.startTopUp("a@example.com", 100);
    REQUIRE_FALSE(result.success);
}

TEST_CASE("startTopUp records a pending order and returns a payment link", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");

    auto result = f.topUp.startTopUp("a@example.com", 500);
    REQUIRE(result.success);
    REQUIRE_FALSE(result.paymentUrl.empty());

    auto order = f.orders.findByReference(result.reference);
    REQUIRE(order.has_value());
    REQUIRE(order->status == persistence::OrderStatus::Pending);
    REQUIRE(order->amountCents == 50000); // KES 500 in cents
    REQUIRE(order->coins == 5000);
}

TEST_CASE("startTopUp rejects bad input", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");

    REQUIRE_FALSE(f.topUp.startTopUp("a@example.com", 5).success);      // too small
    REQUIRE_FALSE(f.topUp.startTopUp("a@example.com", 20000).success);  // too large
    REQUIRE_FALSE(f.topUp.startTopUp("not-an-email", 100).success);
}

TEST_CASE("startTopUp marks the order failed if the provider call fails", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");
    f.gateway.failInitialize = true;

    auto result = f.topUp.startTopUp("a@example.com", 100);
    REQUIRE_FALSE(result.success);
}

TEST_CASE("confirmTopUp credits the wallet exactly once", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");
    auto start = f.topUp.startTopUp("a@example.com", 100);

    auto first = f.topUp.confirmTopUp(start.reference);
    REQUIRE(first.success);
    REQUIRE(first.coinsAdded == 1000);
    REQUIRE(f.wallet.getBalance() == 1000);

    auto second = f.topUp.confirmTopUp(start.reference); // e.g. double click
    REQUIRE(second.success);
    REQUIRE(second.coinsAdded == 0);
    REQUIRE(f.wallet.getBalance() == 1000); // not 2000
}

TEST_CASE("confirmTopUp does not credit an unfinished payment, and can be retried", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");
    auto start = f.topUp.startTopUp("a@example.com", 100);

    f.gateway.statusToReport = payment::ChargeStatus::Pending;
    REQUIRE_FALSE(f.topUp.confirmTopUp(start.reference).success);
    REQUIRE(f.wallet.getBalance() == 0);

    f.gateway.statusToReport = payment::ChargeStatus::Success;
    REQUIRE(f.topUp.confirmTopUp(start.reference).success);
    REQUIRE(f.wallet.getBalance() == 1000);
}

TEST_CASE("confirmTopUp survives the provider being unreachable", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");
    auto start = f.topUp.startTopUp("a@example.com", 100);

    f.gateway.failVerify = true;
    REQUIRE_FALSE(f.topUp.confirmTopUp(start.reference).success);
    REQUIRE(f.orders.findByReference(start.reference)->status == persistence::OrderStatus::Pending);

    f.gateway.failVerify = false;
    REQUIRE(f.topUp.confirmTopUp(start.reference).success);
}

TEST_CASE("confirmTopUp rejects a payment whose amount doesn't match", "[payment]") {
    Fixture f;
    f.signIn("oscar_k");
    auto start = f.topUp.startTopUp("a@example.com", 100);

    f.gateway.amountOverrideCents = 100; // provider reports KES 1, not KES 100
    auto result = f.topUp.confirmTopUp(start.reference);

    REQUIRE_FALSE(result.success);
    REQUIRE(f.wallet.getBalance() == 0);
    REQUIRE(f.orders.findByReference(start.reference)->status == persistence::OrderStatus::Failed);
}

TEST_CASE("confirmTopUp refuses a reference that belongs to another player", "[payment]") {
    Fixture f;
    f.signIn("player_a");
    auto start = f.topUp.startTopUp("a@example.com", 100);

    f.signIn("player_b");
    auto result = f.topUp.confirmTopUp(start.reference);

    REQUIRE_FALSE(result.success);
    REQUIRE(f.wallet.getBalance() == 0);
    REQUIRE(f.orders.findByReference(start.reference)->status == persistence::OrderStatus::Pending);
}