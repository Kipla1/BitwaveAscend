# backend — the C++ game/application core.
#
# Rule: backend must not depend on SFML. Do not add
# target_link_libraries(bitwave_backend PUBLIC/PRIVATE sfml-*) here.
# See docs/ARCHITECTURE.md.
find_package(unofficial-sodium CONFIG REQUIRED)   # libsodium via vcpkg

add_library(bitwave_backend
    src/common/Version.cpp
    src/auth/PasswordHasher.cpp
    src/entites/Player.cpp
    src/auth/AuthServiceImpl.cpp
    src/persistence/Database.cpp
    src/persistence/SqliteUserRepository.cpp
    src/auth/AliasServiceImpl.cpp
    include/auth/AliasServiceImpl.h
    src/persistence/SqlitePlayerProfileRepository.cpp
    src/player/PlayerProfileServiceImpl.cpp
    src/core/GameAPIImpl.cpp
    src/persistence/SqliteWalletRepository.cpp
    src/payment/WalletImpl.cpp
    src/persistence/SqliteStoreItemRepository.cpp
    src/payment/StoreImpl.cpp
    src/persistence/SqliteInventoryRepository.cpp
    src/player/Inventory.cpp
    src/persistence/SqliteTransactionRepository.cpp
)
add_library(bitwave::backend ALIAS bitwave_backend)

target_include_directories(bitwave_backend
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(bitwave_backend
    PUBLIC
        bitwave::shared
    PRIVATE
        unofficial-sodium::sodium
)

target_compile_features(bitwave_backend PUBLIC cxx_std_20)

find_package(SQLite3 REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
target_link_libraries(bitwave_backend 
    PRIVATE
        SQLite3::SQLite3
        unofficial-sodium::sodium
        nlohmann_json::nlohmann_json
        spdlog::spdlog
)
# Intentionally not linked yet: nothing in backend/ uses them during
# repository initialization, and CMake will warn/fail on unused
# find_package() calls in some configurations.
