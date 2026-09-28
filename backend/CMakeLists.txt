# backend — the C++ game/application core.
#
# Rule: backend must not depend on SFML. Do not add
# target_link_libraries(bitwave_backend PUBLIC/PRIVATE sfml-*) here.
# See docs/ARCHITECTURE.md.

add_library(bitwave_backend
    src/common/Version.cpp
    include/auth/PasswordHasher.h
    src/auth/PasswordHasher.cpp
)
add_library(bitwave::backend ALIAS bitwave_backend)

target_include_directories(bitwave_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bitwave_backend
    PUBLIC
        bitwave::shared
)

target_compile_features(bitwave_backend PUBLIC cxx_std_20)

# Future (Day 3+, once the real systems land):
find_package(SQLite3 REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)   # libsodium via vcpkg
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(bitwave_backend 
    PRIVATE
        SQLite::SQLite3
        unofficial-sodium::sodium
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)
# Intentionally not linked yet: nothing in backend/ uses them during
# repository initialization, and CMake will warn/fail on unused
# find_package() calls in some configurations.
