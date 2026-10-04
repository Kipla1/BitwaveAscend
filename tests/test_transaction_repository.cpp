#include <catch2/catch_test_macros.hpp>

#include "persistence/InMemoryTransactionRepository.h"

using namespace bitwave;

TEST_CASE("TransactionRepository records and lists transactions", "[persistence]") {
    persistence::InMemoryTransactionRepository repo;

    SECTION("a recorded transaction can be listed back") {
        auto id = repo.record(1, 42, 50, persistence::TransactionStatus::Succeeded);
        REQUIRE(id > 0);

        auto records = repo.listForUser(1);
        REQUIRE(records.size() == 1);
        REQUIRE(records[0].itemId == 42);
        REQUIRE(records[0].amount == 50);
        REQUIRE(records[0].status == persistence::TransactionStatus::Succeeded);
    }
    SECTION("listForUser only returns that user's transactions") {
        repo.record(1, 1, 50, persistence::TransactionStatus::Succeeded);
        repo.record(2, 1, 50, persistence::TransactionStatus::Succeeded);
        REQUIRE(repo.listForUser(1).size() == 1);
        REQUIRE(repo.listForUser(2).size() == 1);
    }
    SECTION("a user with no transactions gets an empty list") {
        REQUIRE(repo.listForUser(999).empty());
    }
    SECTION("failed status is preserved") {
        repo.record(1, 1, 50, persistence::TransactionStatus::Failed);
        REQUIRE(repo.listForUser(1)[0].status == persistence::TransactionStatus::Failed);
    }
}