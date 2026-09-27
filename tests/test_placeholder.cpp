// Placeholder test confirming the test harness itself is wired up
// correctly. Replace/extend once real systems exist (Day 3+).

#include <catch2/catch_test_macros.hpp>

#include "common/Version.h"

TEST_CASE("backend reports a version string", "[init]") {
    REQUIRE(std::string(bitwave::common::backendVersion()) == "0.1.0-init");
}
