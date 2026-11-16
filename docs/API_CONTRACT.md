# API Contract

This document describes the frontend/backend contract defined in
`shared/include/shared/`. Nothing here depends on SFML or SQLite — that is
the point of putting it in `shared/` rather than `backend/` or `frontend/`.

## Enums

```cpp
enum class GameModeType {
    Fighting,
    Running,
    Flappy
};

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

enum class Difficulty { Easy, Medium, Hard };
```

## GameState

Snapshot of gameplay state the frontend needs to render a frame. Backend
owns it; frontend treats it as read-only.

| Field       | Type        | Meaning |
|-------------|-------------|---------|
| `playerX`   | `float`     | Player X position |
| `playerY`   | `float`     | Player Y position |
| `velocityX` | `float`     | Player X velocity |
| `velocityY` | `float`     | Player Y velocity |
| `health`    | `int`       | Current health |
| `score`     | `long long` | Current score |
| `gameOver`  | `bool`      | Whether the run has ended |
| `paused`    | `bool`      | Whether simulation is paused |
| `playerSkinId` | `int64_t` | Equipped cosmetic ID; `0` is the free built-in skin |

## PlayerProfile

Public-facing profile (no password hash, no session token).

| Field          | Type          |
|----------------|---------------|
| `userId`       | `int64_t`     |
| `username`     | `string`      |
| `alias`        | `string`      |
| `highestScore` | `long long`   |
| `currentLevel` | `int`         |
| `equippedSkinId` | `int64_t`    | Owned cosmetic ID; `0` is the built-in skin |

## LeaderboardEntry

| Field | Type | Meaning |
|-------|------|---------|
| `rank` | `int` | Position, starting at 1 |
| `alias` | `string` | Public player display name |
| `score` | `long long` | Saved highest score |

## StoreItem

| Field         | Type        |
|---------------|-------------|
| `itemId`      | `int64_t`   |
| `name`        | `string`    |
| `price`       | `long long` |
| `description` | `string`    |
| `kind`        | `StoreItemKind` | Power-up or cosmetic |
| `resourceKey` | `string`    | Frontend asset lookup key |

`startMode(mode, difficulty)` selects easy, medium, or hard tuning. Movement
actions are held until sent again with `pressed=false`; jump, attack, and flap
are triggered on press.

`getCosmetics()` returns each skin's catalog data, ownership, and equipped
status. `equipCosmetic(id)` succeeds only for an owned skin. The built-in skin
is always available. Purchases use the virtual wallet and save ownership in
inventory; duplicate cosmetic purchases are rejected. Cosmetics never gate a
game mode or alter gameplay.

## Result Objects

- `LoginResult { success, message, profile }`
- `RegisterResult { success, message }`
- `PurchaseResult { success, item, amount, message }`

## GameAPI

The only boundary the frontend is allowed to call through:

```cpp
class GameAPI {
public:
    LoginResult login(const std::string& username, const std::string& password);
    RegisterResult registerUser(const std::string& username, const std::string& password);
    void logout();

    std::optional<PlayerProfile> getProfile();
    bool setAlias(const std::string& alias);
    std::vector<LeaderboardEntry> getLeaderboard(std::size_t limit = 10);

    void startMode(GameModeType mode, Difficulty difficulty = Difficulty::Medium);
    void handleAction(PlayerAction action, bool pressed = true);
    void update(float deltaTime);
    GameState getGameState();

    long long getWalletBalance();
    std::vector<StoreItem> getStoreItems();
    PurchaseResult purchaseItem(std::int64_t itemId);
    std::vector<CosmeticItem> getCosmetics();
    bool equipCosmetic(std::int64_t itemId);
};
```

Data flow: `Frontend → GameAPI → Backend Systems → Persistence`. Never
`Frontend → SQLite`, never `Frontend → internal backend classes`.

`GameAPI` is the abstract interface implemented by `GameAPIImpl`. Gameplay and store methods need their matching services configured. Alias and leaderboard calls need `AliasService` and `LeaderboardRepository` configured. The leaderboard returns at most 100 entries.

`getProfile()` returns `std::nullopt` when no one is logged in. The frontend
should treat this as "route to the login screen," not as an error.

Any change to this contract must be flagged to the whole team before
merging (see `docs/CONTRIBUTING.md`).
