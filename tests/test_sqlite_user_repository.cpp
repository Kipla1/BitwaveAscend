#include <catch2/catch_test_macros.hpp>

#include "auth/AuthServiceImpl.h"
#include "auth/FakeGoogleAuthProvider.h"
#include "auth/PasswordHasher.h"
#include "auth/Session.h"
#include "persistence/Database.h"
#include "persistence/SqlitePlayerProfileRepository.h"
#include "persistence/SqliteUserRepository.h"
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
    SECTION("Google user has empty passwordHash and can be found by google_sub") {
        auto id = repo.createGoogleUser("oscar_g", "oscar@example.com", "sub-123");
        REQUIRE(id.has_value());

        auto bySub = repo.findByGoogleSub("sub-123");
        REQUIRE(bySub.has_value());
        REQUIRE(bySub->id == *id);
        REQUIRE(bySub->username == "oscar_g");
        REQUIRE(bySub->email == "oscar@example.com");
        REQUIRE(bySub->googleSub == "sub-123");
        REQUIRE(bySub->passwordHash.empty());
    }
    SECTION("duplicate google_sub is rejected") {
        REQUIRE(repo.createGoogleUser("oscar_1", "a@example.com", "same-sub").has_value());
        REQUIRE_FALSE(repo.createGoogleUser("oscar_2", "b@example.com", "same-sub").has_value());
    }
}

TEST_CASE("SqliteUserRepository migrates legacy users table with NOT NULL password_hash", "[persistence]") {
    persistence::Database db(":memory:");
    db.execute(
        "CREATE TABLE users ("
        "  id            INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  username      TEXT    NOT NULL UNIQUE,"
        "  password_hash TEXT    NOT NULL,"
        "  alias         TEXT    NOT NULL UNIQUE,"
        "  created_at    TEXT    NOT NULL DEFAULT CURRENT_TIMESTAMP"
        ");"
        "INSERT INTO users (username, password_hash, alias) VALUES ('legacy_user', 'hash1', 'legacy_user');");

    persistence::SqliteUserRepository repo(db);

    auto legacy = repo.findByUsername("legacy_user");
    REQUIRE(legacy.has_value());
    REQUIRE(legacy->passwordHash == "hash1");

    auto googleId = repo.createGoogleUser("google_user", "g@example.com", "sub-migrated");
    REQUIRE(googleId.has_value());
}

TEST_CASE("AuthServiceImpl works unchanged on the SQLite repository", "[auth][persistence]") {
    persistence::Database db(":memory:");
    persistence::SqliteUserRepository repo(db);
    persistence::SqlitePlayerProfileRepository progress(db);
    persistence::SqliteWalletRepository wallets(db);
    auth::PasswordHasher hasher;
    auth::Session session;
    auth::FakeGoogleAuthProvider google;
    auth::AuthServiceImpl service(repo, progress, wallets, hasher, session, google);

    REQUIRE(service.registerUser("oscar_k", "longenough1").success);
    REQUIRE_FALSE(service.registerUser("oscar_k", "longenough1").success);
    REQUIRE(service.login("oscar_k", "longenough1").success);
    REQUIRE_FALSE(service.login("oscar_k", "wrongpassword").success);

    service.logout();
    google.googleSub = "sqlite-sub-1";
    google.email = "sqlite_player@example.com";
    auto gLogin = service.loginWithGoogle();
    REQUIRE(gLogin.success);
    service.logout();
    REQUIRE_FALSE(service.login(gLogin.profile.username, "longenough1").success);
}