#pragma once

// Frontend/backend shared contract types.
//
// Everything in this header is owned jointly by backend and frontend
// developers. Breaking changes here must be communicated to the whole team
// (see docs/CONTRIBUTING.md). Nothing in this file talks to SFML or SQLite.

#include <cstdint>
#include <string>

namespace bitwave::shared {

enum class GameModeType {
    Fighting,
    Running,
    Flappy
};

enum class Difficulty { Easy, Medium, Hard };

enum class PlayerAction {
    MoveLeft,
    MoveRight,
    MoveForward,
    MoveBackward,
    Jump,
    Attack,
    Flap,
    Pause
};

enum class StoreItemKind { PowerUp, Cosmetic };

// Snapshot of gameplay state the frontend needs in order to render a frame.
// Populated by the backend; the frontend must treat it as read-only.
struct GameState {
    float playerX = 0.0f;
    float playerY = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    int   health = 0;
    long long score = 0;
    bool  gameOver = false;
    bool  paused = false;
    std::int64_t playerSkinId = 0; // 0 is the free built-in skin.
};

// Public-facing player profile. Deliberately excludes anything sensitive
// (password hashes, session tokens, etc.).
struct PlayerProfile {
    std::int64_t userId = 0;
    std::string  username;
    std::string  alias;
    long long    highestScore = 0;
    int          currentLevel = 0;
    std::int64_t equippedSkinId = 0;
};

struct LeaderboardEntry {
    int rank = 0;
    std::string alias;
    long long score = 0;
};

struct StoreItem {
    std::int64_t itemId = 0;
    std::string  name;
    long long    price = 0;
    std::string  description;
    StoreItemKind kind = StoreItemKind::PowerUp;
    std::string  resourceKey; // Frontend asset key, not a file path.
};

struct CosmeticItem {
    std::int64_t itemId = 0;
    std::string name;
    std::string resourceKey;
    long long price = 0;
    bool owned = false;
    bool equipped = false;
};

struct PurchaseResult {
    bool         success = false;
    StoreItem    item;
    long long    amount = 0;
    std::string  message;
};

struct LoginResult {
    bool         success = false;
    std::string  message;
    PlayerProfile profile;
};

struct RegisterResult {
    bool         success = false;
    std::string  message;
};

} // namespace bitwave::shared
