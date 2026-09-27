# shared — the frontend/backend contract.
# Header-only on purpose: it must stay free of both SFML and SQLite so that
# either side can include it without pulling in the other's dependencies.

add_library(bitwave_shared INTERFACE)
add_library(bitwave::shared ALIAS bitwave_shared)

target_include_directories(bitwave_shared
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(bitwave_shared INTERFACE cxx_std_20)
