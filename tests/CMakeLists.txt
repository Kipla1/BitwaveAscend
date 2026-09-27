# tests — Catch2-based backend test suite.
#
# Only enabled when -DBITWAVE_BUILD_TESTS=ON (see root CMakeLists.txt).
# Backend logic must stay testable without an SFML window (see
# docs/ARCHITECTURE.md, section "Testing Strategy").

find_package(Catch2 3 QUIET)

if(NOT Catch2_FOUND)
    message(WARNING
        "Bitwave Ascend: Catch2 3 not found — test target skipped. "
        "Install Catch2 (e.g. via vcpkg) and re-run CMake to build tests."
    )
    return()
endif()

add_executable(bitwave_tests
    test_placeholder.cpp
    # test_auth.cpp          # Day 3+
    # test_wallet.cpp        # Day 10+
    # test_game_modes.cpp    # Day 5+
    # test_collision.cpp     # Day 6+
    # test_persistence.cpp   # Day 3-11
)

target_link_libraries(bitwave_tests
    PRIVATE
        bitwave::backend
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(bitwave_tests)
