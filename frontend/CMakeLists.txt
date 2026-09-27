# frontend — SFML rendering, input, UI, audio.
#
# Only enabled when -DBITWAVE_BUILD_FRONTEND=ON (see root CMakeLists.txt).
# Frontend must not access SQLite, wallet internals, auth internals,
# repositories or password hashes directly — only bitwave::shared and the
# future GameAPI. See docs/ARCHITECTURE.md.

find_package(SFML 3 COMPONENTS Graphics Window System Audio QUIET)

if(NOT SFML_FOUND)
    message(WARNING
        "Bitwave Ascend: SFML 3 not found — frontend target skipped. "
        "Install SFML (e.g. via vcpkg) and re-run CMake to build it."
    )
    return()
endif()

# Placeholder executable: intentionally does nothing beyond opening/closing.
# Real menus/rendering/input arrive on Day 2+.
add_executable(bitwave_frontend
    src/main.cpp
)

target_include_directories(bitwave_frontend
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bitwave_frontend
    PRIVATE
        bitwave::shared
        SFML::Graphics
        SFML::Window
        SFML::System
        SFML::Audio
)

target_compile_features(bitwave_frontend PRIVATE cxx_std_20)
