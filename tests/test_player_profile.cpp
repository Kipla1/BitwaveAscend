#include <catch2/catch_test_macros.hpp>

#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "persistence/Database.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteUserRepository.h"
#include "player/PlayerProfileServiceImpl.h"
#include "persistence/SqliteWalletRepository.h"

using namespace bitwave;

TEST_CASE("PlayerProfileService reflects registration and reported progress", "[player]") {
    persistence::Database db(":memory:");
    persistence::SqliteUserRepository users(db);
    persistence::SqlitePlayerProfileRepository progress(db);
    persistence::SqliteWalletRepository wallets(db);
    auth::PasswordHasher hasher;
    auth::Session session;
    auth::AuthServiceImpl authService(users, progress, wallets, hasher, session);
    player::PlayerProfileServiceImpl profileService(progress, session);
    SECTION("no profile before login") {
        REQUIRE_FALSE(profileService.getCurrentProfile().has_value());
    }

    authService.registerUser("oscar_k", "longenough1");
    authService.login("oscar_k", "longenough1");

    SECTION("fresh registration starts at zero") {
        auto profile = profileService.getCurrentProfile();
        REQUIRE(profile.has_value());
        REQUIRE(profile->highestScore == 0);
        REQUIRE(profile->currentLevel == 0);
    }
    SECTION("reportScore raises highest_score") {
        profileService.reportScore(500);
        REQUIRE(profileService.getCurrentProfile()->highestScore == 500);
    }
    SECTION("reportScore never lowers highest_score") {
        profileService.reportScore(500);
        profileService.reportScore(100);
        REQUIRE(profileService.getCurrentProfile()->highestScore == 500);
    }
    SECTION("setLevel updates current_level") {
        profileService.setLevel(3);
        REQUIRE(profileService.getCurrentProfile()->currentLevel == 3);
    }
}