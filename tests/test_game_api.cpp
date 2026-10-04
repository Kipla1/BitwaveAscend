#include <catch2/catch_test_macros.hpp>

#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "core/GameAPIImpl.h"
#include "persistence/InMemoryPlayerProfileRepository.h"
#include "persistence/InMemoryUserRepository.h"
#include "player/PlayerProfileServiceImpl.h"
#include "persistence/InMemoryWalletRepository.h"

using namespace bitwave;

namespace {

struct Fixture {
    persistence::InMemoryUserRepository users;
    persistence::InMemoryPlayerProfileRepository progress;
    persistence::InMemoryWalletRepository wallets;
    auth::PasswordHasher hasher;
    auth::Session session;
    auth::AuthServiceImpl authService{users, progress, wallets, hasher, session};
    player::PlayerProfileServiceImpl profileService{progress, session};
    core::GameAPIImpl api{authService, profileService};
};

} // namespace

TEST_CASE("GameAPIImpl register, login and getProfile work end-to-end", "[core]") {
    Fixture f;

    REQUIRE(f.api.registerUser("oscar_k", "longenough1").success);

    auto loginResult = f.api.login("oscar_k", "longenough1");
    REQUIRE(loginResult.success);

    auto profile = f.api.getProfile();
    REQUIRE(profile.has_value());
    REQUIRE(profile->username == "oscar_k");
    REQUIRE(profile->highestScore == 0);
}

TEST_CASE("GameAPIImpl getProfile is empty when nobody is logged in", "[core]") {
    Fixture f;
    REQUIRE_FALSE(f.api.getProfile().has_value());
}

TEST_CASE("GameAPIImpl logout clears the session via AuthService", "[core]") {
    Fixture f;
    f.api.registerUser("oscar_k", "longenough1");
    f.api.login("oscar_k", "longenough1");
    f.api.logout();
    REQUIRE_FALSE(f.api.getProfile().has_value());
}

TEST_CASE("GameAPIImpl's unimplemented methods fail loudly, not silently", "[core]") {
    Fixture f;
    // Documents intent: calling these before GameManager/Wallet exist is a
    // programming error, not a valid no-op. If this test ever fails because
    // startMode no longer throws, it means someone implemented it — update
    // this test to reflect real behavior instead of deleting it.
    REQUIRE_THROWS_AS(f.api.startMode(shared::GameModeType::Fighting), std::logic_error);
    REQUIRE_THROWS_AS(f.api.getWalletBalance(), std::logic_error);
}