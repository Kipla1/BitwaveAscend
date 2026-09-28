#include <catch2/catch_test_macros.hpp>

#include "auth/AuthServiceImpl.h"
#include "auth/PasswordHasher.h"
#include "persistence/InMemoryUserRepository.h"

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
    auth::PasswordHasher hasher;
    auth::AuthServiceImpl service(repo, hasher);

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
    auth::PasswordHasher hasher;
    auth::AuthServiceImpl service(repo, hasher);
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
    SECTION("logout does not crash") {
        service.login("oscar_k", "longenough1");
        service.logout();
        SUCCEED();
    }
}