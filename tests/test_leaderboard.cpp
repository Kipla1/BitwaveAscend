#include <catch2/catch_test_macros.hpp>

#include "persistence/Database.h"
#include "persistence/SqliteLeaderboardRepository.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteUserRepository.h"

using namespace bitwave;

TEST_CASE("SQLite leaderboard returns persisted scores and aliases in rank order", "[leaderboard]") {
    persistence::Database db(":memory:");
    persistence::SqliteUserRepository users(db);
    persistence::SqlitePlayerProfileRepository profiles(db);
    persistence::SqliteLeaderboardRepository leaderboard(db);

    const auto first=users.create("player_one", "hash");
    const auto second=users.create("player_two", "hash");
    REQUIRE(first);
    REQUIRE(second);
    profiles.create(*first);
    profiles.create(*second);
    profiles.updateHighestScore(*first, 120);
    profiles.updateHighestScore(*second, 300);
    REQUIRE(users.updateAlias(*second, "SkyRider"));

    const auto rows=leaderboard.top(10);
    REQUIRE(rows.size()==2);
    CHECK(rows[0].rank==1);
    CHECK(rows[0].alias=="SkyRider");
    CHECK(rows[0].score==300);
    CHECK(rows[1].rank==2);
    CHECK(rows[1].score==120);
}
