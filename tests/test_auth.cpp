#include <catch2/catch_test_macros.hpp>

#include "auth/Session.h"
#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "persistence/InMemoryUserRepository.h"
#include "persistence/InMemoryPlayerProfileRepository.h"
#include "persistence/InMemoryWalletRepository.h"

using namespace bitwave;

TEST_CASE("PasswordHasher verifies correct and rejects wrong passwords", "[auth]") {
    auth::PasswordHasher hasher;
    const std::string hash = hasher.hash("correct horse");

    REQUIRE_FALSE(hash.empty());
    REQUIRE(hasher.verify(hash, "correct horse"));
    REQUIRE_FALSE(hasher.verify(hash, "wrong horse"));
}

TEST_CASE("PasswordHasher salts each hash", "[auth]") {
    auth::PasswordHasher hasher;
    REQUIRE(hasher.hash("same password") != hasher.hash("same password"));
}

TEST_CASE("AuthService registration", "[auth]") {
    persistence::InMemoryUserRepository repo;
    persistence::InMemoryPlayerProfileRepository progress;
    auth::PasswordHasher hasher;
    auth::Session session;
    persistence::InMemoryWalletRepository wallets;
    auth::AuthServiceImpl service(repo, progress, wallets, hasher, session);

    SECTION("valid registration succeeds") {
        REQUIRE(service.registerUser("oscar_k", "longenough1").success);
    }
    SECTION("duplicate username is rejected") {
        REQUIRE(service.registerUser("oscar_k", "longenough1").success);
        REQUIRE_FALSE(service.registerUser("oscar_k", "another-pass1").success);
    }
    SECTION("short password is rejected") {
        REQUIRE_FALSE(service.registerUser("oscar_k", "short").success);
    }
    SECTION("invalid username is rejected") {
        REQUIRE_FALSE(service.registerUser("no spaces!", "longenough1").success);
    }
}

TEST_CASE("AuthService login", "[auth]") {
    persistence::InMemoryUserRepository repo;
    persistence::InMemoryPlayerProfileRepository progress;
    auth::PasswordHasher hasher;
    auth::Session session;
    persistence::InMemoryWalletRepository wallets;
    auth::AuthServiceImpl service(repo, progress, wallets, hasher, session);
    service.registerUser("oscar_k", "longenough1");

    SECTION("correct credentials return the profile") {
        auto result = service.login("oscar_k", "longenough1");
        REQUIRE(result.success);
        REQUIRE(result.profile.username == "oscar_k");
    }
    SECTION("wrong password and unknown user give identical messages") {
        auto wrongPassword = service.login("oscar_k", "wrongpassword");
        auto unknownUser   = service.login("nobody", "longenough1");
        REQUIRE_FALSE(wrongPassword.success);
        REQUIRE_FALSE(unknownUser.success);
        REQUIRE(wrongPassword.message == unknownUser.message);
    }
    SECTION("successful login starts the session") {
        REQUIRE_FALSE(session.isActive());
        auto result = service.login("oscar_k", "longenough1");
        REQUIRE(session.isActive());
        REQUIRE(session.user()->id == result.profile.userId);
        REQUIRE(session.user()->username == "oscar_k");
    }
    SECTION("failed login leaves the session inactive") {
        service.login("oscar_k", "wrongpassword");
        REQUIRE_FALSE(session.isActive());
    }
    SECTION("failed login does not log out the current player") {
        service.login("oscar_k", "longenough1");
        service.login("oscar_k", "wrongpassword");
        REQUIRE(session.isActive());
    }
    SECTION("logout ends the session") {
        service.login("oscar_k", "longenough1");
        service.logout();
        REQUIRE_FALSE(session.isActive());
    }
}

TEST_CASE("Session starts empty and can be started and ended", "[auth]") {
    auth::Session session;
    REQUIRE_FALSE(session.isActive());
    session.start({7, "oscar_k", "oscar_k"});
    REQUIRE(session.isActive());
    REQUIRE(session.user()->id == 7);
    session.end();
    REQUIRE_FALSE(session.user().has_value());
}