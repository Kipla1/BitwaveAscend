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

enum class PlayerAction {
    MoveLeft,
    MoveRight,
    Jump,
    Attack,
    Flap,
    Pause
};

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
};

// Public-facing player profile. Deliberately excludes anything sensitive
// (password hashes, session tokens, etc.).
struct PlayerProfile {
    std::int64_t userId = 0;
    std::string  username;
    std::string  alias;
    long long    highestScore = 0;
    int          currentLevel = 0;
};

struct StoreItem {
    std::int64_t itemId = 0;
    std::string  name;
    long long    price = 0;
    std::string  description;
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
