#include <catch2/catch_test_macros.hpp>

#include "player/Inventory.h"
#include "persistence/InMemoryInventoryRepository.h"

using namespace bitwave;

TEST_CASE("Inventory behavior with an active session", "[player]") {
    persistence::InMemoryInventoryRepository repo;
    auth::Session session;
    session.start({1, "oscar_k", "oscar_k"});
    player::InventoryImpl inventory(repo, session);

    SECTION("starts empty") {
        REQUIRE(inventory.listItems().empty());
    }
    SECTION("addItem creates a new entry") {
        inventory.addItem(/*itemId=*/1, 3);
        auto items = inventory.listItems();
        REQUIRE(items.size() == 1);
        REQUIRE(items[0].itemId == 1);
        REQUIRE(items[0].quantity == 3);
    }
    SECTION("addItem on an existing entry increments quantity") {
        inventory.addItem(1, 3);
        inventory.addItem(1, 2);
        REQUIRE(inventory.listItems()[0].quantity == 5);
    }
    SECTION("useItem succeeds and decrements quantity") {
        inventory.addItem(1, 3);
        REQUIRE(inventory.useItem(1, 2));
        REQUIRE(inventory.listItems()[0].quantity == 1);
    }
    SECTION("useItem fails without enough quantity, nothing changes") {
        inventory.addItem(1, 1);
        REQUIRE_FALSE(inventory.useItem(1, 5));
        REQUIRE(inventory.listItems()[0].quantity == 1);
    }
    SECTION("useItem fails for an item never owned") {
        REQUIRE_FALSE(inventory.useItem(999, 1));
    }
}

TEST_CASE("Inventory with no active session is inert, not a crash", "[player]") {
    persistence::InMemoryInventoryRepository repo;
    auth::Session session;
    player::InventoryImpl inventory(repo, session);

    REQUIRE(inventory.listItems().empty());
    REQUIRE_FALSE(inventory.useItem(1, 1));
}