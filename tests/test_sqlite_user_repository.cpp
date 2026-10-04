#include <catch2/catch_test_macros.hpp>

#include "auth/Session.h"
#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "persistence/Database.h"
#include "persistence/SqliteUserRepository.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteWalletRepository.h"

using namespace bitwave;

TEST_CASE("SqliteUserRepository stores and finds users", "[persistence]") {
    persistence::Database db(":memory:");
    persistence::SqliteUserRepository repo(db);

    SECTION("created user can be found") {
        auto id = repo.create("oscar_k", "hash-value");
        REQUIRE(id.has_value());

        auto found = repo.findByUsername("oscar_k");
        REQUIRE(found.has_value());
        REQUIRE(found->id == *id);
        REQUIRE(found->passwordHash == "hash-value");
    }
    SECTION("unknown user returns nullopt") {
        REQUIRE_FALSE(repo.findByUsername("nobody").has_value());
    }
    SECTION("duplicate username returns nullopt") {
        REQUIRE(repo.create("oscar_k", "h1").has_value());
        REQUIRE_FALSE(repo.create("oscar_k", "h2").has_value());
    }
    SECTION("SQL injection text is treated as plain data") {
        repo.create("oscar_k", "h1");
        REQUIRE_FALSE(repo.findByUsername("' OR '1'='1").has_value());
        REQUIRE(repo.create("x'); DROP TABLE users;--", "h").has_value());
        REQUIRE(repo.findByUsername("oscar_k").has_value()); // table still intact
    }
}

TEST_CASE("AuthServiceImpl works unchanged on the SQLite repository", "[auth][persistence]") {
    persistence::Database db(":memory:");
    persistence::SqliteUserRepository repo(db);
    persistence::SqlitePlayerProfileRepository progress(db);
    persistence::SqliteWalletRepository wallets(db);
    auth::PasswordHasher hasher;
    auth::Session session;
    auth::AuthServiceImpl service(repo, progress, wallets, hasher, session);

    REQUIRE(service.registerUser("oscar_k", "longenough1").success);
    REQUIRE_FALSE(service.registerUser("oscar_k", "longenough1").success);
    REQUIRE(service.login("oscar_k", "longenough1").success);
    REQUIRE_FALSE(service.login("oscar_k", "wrongpassword").success);
}