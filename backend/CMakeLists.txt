# backend — the C++ game/application core.
#
# Rule: backend must not depend on SFML. Do not add
# target_link_libraries(bitwave_backend PUBLIC/PRIVATE sfml-*) here.
# See docs/ARCHITECTURE.md.
find_package(unofficial-sodium CONFIG REQUIRED)   # libsodium via vcpkg
find_package(SQLite3 REQUIRED)

add_library(bitwave_backend
    src/common/Version.cpp
    src/auth/PasswordHasher.cpp
    src/entities/Entity.cpp
    src/entities/Character.cpp
    src/entities/Player.cpp
    src/entities/Enemy.cpp
    src/auth/AuthServiceImpl.cpp
    src/persistence/Database.cpp
    src/persistence/SqliteUserRepository.cpp
    src/persistence/SqliteLeaderboardRepository.cpp
    src/auth/AliasServiceImpl.cpp
    include/auth/AliasServiceImpl.h
    src/persistence/SqlitePlayerProfileRepository.cpp
    src/player/PlayerProfileServiceImpl.cpp
    src/core/GameAPIImpl.cpp
    src/core/Game.cpp
    src/core/GameMode.cpp
    src/core/GameManager.cpp
    src/gameplay/Physics.cpp
    src/gameplay/Combat.cpp
    src/gameplay/FightingMode.cpp
    src/gameplay/RunningMode.cpp
    src/gameplay/FlappyMode.cpp
    src/persistence/SqliteWalletRepository.cpp
    src/payment/WalletImpl.cpp
    src/persistence/SqliteStoreItemRepository.cpp
    src/payment/StoreImpl.cpp
    src/persistence/SqliteInventoryRepository.cpp
    src/player/Inventory.cpp
    src/persistence/SqliteTransactionRepository.cpp
    src/payment/PaymentServiceImpl.cpp
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
        SQLite3::SQLite3
)

target_compile_features(bitwave_backend PUBLIC cxx_std_20)
