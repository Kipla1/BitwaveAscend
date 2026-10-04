#include <catch2/catch_test_macros.hpp>

#include "payment/StoreImpl.h"
#include "persistence/InMemoryStoreItemRepository.h"

using namespace bitwave;

TEST_CASE("Store lists and finds items", "[payment]") {
    persistence::InMemoryStoreItemRepository repo;
    repo.addItem({1, "Health Potion", 50, "Restores health"});
    repo.addItem({2, "Speed Boost", 100, "Temporary speed increase"});
    payment::StoreImpl store(repo);

    SECTION("listItems returns everything") {
        auto items = store.listItems();
        REQUIRE(items.size() == 2);
    }
    SECTION("getItem finds an existing item") {
        auto item = store.getItem(1);
        REQUIRE(item.has_value());
        REQUIRE(item->name == "Health Potion");
    }
    SECTION("getItem returns nullopt for an unknown id") {
        REQUIRE_FALSE(store.getItem(999).has_value());
    }
}

TEST_CASE("Store with no items returns an empty list, not an error", "[payment]") {
    persistence::InMemoryStoreItemRepository repo;
    payment::StoreImpl store(repo);
    REQUIRE(store.listItems().empty());
}