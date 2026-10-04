#include <catch2/catch_test_macros.hpp>

#include "payment/WalletImpl.h"
#include "persistence/InMemoryWalletRepository.h"

using namespace bitwave;

TEST_CASE("Wallet behavior with an active session", "[payment]") {
    persistence::InMemoryWalletRepository repo;
    repo.create(1);
    auth::Session session;
    session.start({1, "oscar_k", "oscar_k"});
    payment::WalletImpl wallet(repo, session);

    SECTION("starts at zero") {
        REQUIRE(wallet.getBalance() == 0);
    }
    SECTION("addFunds increases balance") {
        wallet.addFunds(100);
        REQUIRE(wallet.getBalance() == 100);
    }
    SECTION("spend succeeds with sufficient funds") {
        wallet.addFunds(100);
        REQUIRE(wallet.spend(80));
        REQUIRE(wallet.getBalance() == 20);
    }
    SECTION("spend fails with insufficient funds, balance unchanged") {
        wallet.addFunds(50);
        REQUIRE_FALSE(wallet.spend(80));
        REQUIRE(wallet.getBalance() == 50);
    }
    SECTION("spend never allows a negative balance") {
        wallet.addFunds(10);
        REQUIRE_FALSE(wallet.spend(11));
        REQUIRE(wallet.getBalance() == 10);
    }
}

TEST_CASE("Wallet with no active session is inert, not a crash", "[payment]") {
    persistence::InMemoryWalletRepository repo;
    auth::Session session; // never started
    payment::WalletImpl wallet(repo, session);

    REQUIRE(wallet.getBalance() == 0);
    REQUIRE_FALSE(wallet.spend(1));
}