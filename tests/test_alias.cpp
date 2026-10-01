#include <catch2/catch_test_macros.hpp>

#include "./auth/AliasServiceImpl.h"
#include "persistence/InMemoryUserRepository.h"

using namespace bitwave;

TEST_CASE("AliasService requires an active session", "[auth]") {
    persistence::InMemoryUserRepository repo;
    auth::Session session;
    auth::AliasServiceImpl aliasService(repo, session);

    REQUIRE_FALSE(aliasService.setAlias("NewName"));
}

TEST_CASE("AliasService changes and validates the alias", "[auth]") {
    persistence::InMemoryUserRepository repo;
    auto userId = repo.create("oscar_k", "hash");
    REQUIRE(userId.has_value());

    auth::Session session;
    session.start({*userId, "oscar_k", "oscar_k"});
    auth::AliasServiceImpl aliasService(repo, session);

    SECTION("valid alias is accepted and updates the session") {
        REQUIRE(aliasService.setAlias("ShadowKing"));
        REQUIRE(session.user()->alias == "ShadowKing");
    }
    SECTION("alias with invalid characters is rejected") {
        REQUIRE_FALSE(aliasService.setAlias("no spaces!"));
    }
    SECTION("alias that's too short is rejected") {
        REQUIRE_FALSE(aliasService.setAlias("ab"));
    }
    SECTION("taken alias is rejected") {
        repo.create("other_user", "hash2");
        auth::Session otherSession;
        otherSession.start({2, "other_user", "other_user"});
        auth::AliasServiceImpl otherAliasService(repo, otherSession);
        REQUIRE(otherAliasService.setAlias("ShadowKing"));

        REQUIRE_FALSE(aliasService.setAlias("ShadowKing"));
    }
}