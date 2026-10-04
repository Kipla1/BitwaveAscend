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
    Jump,
    Attack,
    Flap,
    Pause
};
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

## PlayerProfile

Public-facing profile (no password hash, no session token).

| Field          | Type          |
|----------------|---------------|
| `userId`       | `int64_t`     |
| `username`     | `string`      |
| `alias`        | `string`      |
| `highestScore` | `long long`   |
| `currentLevel` | `int`         |

## StoreItem

| Field         | Type        |
|---------------|-------------|
| `itemId`      | `int64_t`   |
| `name`        | `string`    |
| `price`       | `long long` |
| `description` | `string`    |

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

    void startMode(GameModeType mode);
    void handleAction(PlayerAction action);
    void update(float deltaTime);
    GameState getGameState();

    long long getWalletBalance();
    std::vector<StoreItem> getStoreItems();
    PurchaseResult purchaseItem(int itemId);
};
```

Data flow: `Frontend → GameAPI → Backend Systems → Persistence`. Never
`Frontend → SQLite`, never `Frontend → internal backend classes`.

`GameAPI` is declared as an abstract interface in
`shared/include/shared/GameAPI.h`. Not every method needs a working
implementation yet — see `docs/DEVELOPMENT_PLAN.md` for which day
introduces which piece (Day 2: interfaces frozen; Day 3: auth; Day 5:
mode switching; Day 10: wallet/store).

`getProfile()` returns `std::nullopt` when no one is logged in. The frontend
should treat this as "route to the login screen," not as an error.

Any change to this contract must be flagged to the whole team before
merging (see `docs/CONTRIBUTING.md`).