cmake_minimum_required(VERSION 3.21)

project(BitwaveAscend
    VERSION 0.1.0
    DESCRIPTION "Bitwave Ascend - university OOP 2D game project"
    LANGUAGES CXX
)

# ---------------------------------------------------------------------------
# Global project settings
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif()

# Options let each piece be toggled independently. Frontend defaults to OFF
# during repository initialization because SFML may not be installed on
# every machine yet (see docs/DEVELOPMENT_PLAN.md, Day 1 vs Day 2).
option(BITWAVE_BUILD_FRONTEND "Build the SFML frontend target" OFF)
option(BITWAVE_BUILD_TESTS    "Build the Catch2 test targets"   OFF)

# Keep dependency lookups localized to this project.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------
add_subdirectory(shared)
add_subdirectory(backend)

if(BITWAVE_BUILD_FRONTEND)
    add_subdirectory(frontend)
else()
    message(STATUS "Bitwave Ascend: frontend disabled (BITWAVE_BUILD_FRONTEND=OFF)")
endif()

if(BITWAVE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
else()
    message(STATUS "Bitwave Ascend: tests disabled (BITWAVE_BUILD_TESTS=OFF)")
endif()

message(STATUS "Bitwave Ascend ${PROJECT_VERSION} configured (C++${CMAKE_CXX_STANDARD})")
