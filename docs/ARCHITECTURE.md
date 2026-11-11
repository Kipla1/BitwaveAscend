# Architecture

## Frontend / Backend Separation

Bitwave Ascend is not a web application. The **backend is the C++
game/application core** (not a server); the **frontend is SFML**
(rendering, input, UI, audio).

> Backend determines WHAT exists and WHAT happens.
> Frontend determines HOW it looks and HOW the user interacts with it.

```text
                    ┌─────────────────────┐
                    │      FRONTEND       │
                    │        SFML         │
                    │ Rendering / Input   │
                    │ UI / Animation      │
                    │ Audio               │
                    └──────────┬──────────┘
                               │
                               │ GameAPI
                               ▼
                    ┌─────────────────────┐
                    │ APPLICATION / CORE  │
                    │ Game / GameState    │
                    │ GameManager         │
                    │ GameMode            │
                    └──────────┬──────────┘
                               │
              ┌────────────────┼────────────────┐
              ▼                ▼                ▼
         GAMEPLAY          SERVICES         PERSISTENCE
         SYSTEMS           SYSTEMS              │
              │                │                ▼
              ▼                ▼              SQLite
          Entities       Auth / Wallet
          Combat         Payment / Store
          Physics        Inventory
          Collision      Profile
```

Rule: `backend/` must never `#include <SFML/...>`. Frontend must never
touch SQLite, wallet internals, authentication internals, repositories, or
password hashes directly.

## Backend Modules

| Module        | Path                            | Owns |
|---------------|----------------------------------|------|
| `core`        | `backend/include/core/`         | `Game`, `GameState`, `GameManager`, `GameMode` |
| `entities`    | `backend/include/entities/`     | `Entity`, `Player`, `Enemy`, `Obstacle` |
| `gameplay`    | `backend/include/gameplay/`     | `FightingMode`, `RunningMode`, `FlappyMode`, `Collision`, `Combat`, `Physics` |
| `auth`        | `backend/include/auth/`         | `AuthService`, `Session`, `AliasService` |
| `player`      | `backend/include/player/`       | `PlayerProfile`, `Inventory` |
| `payment`     | `backend/include/payment/`      | `Wallet`, `PaymentService`, `Store` |
| `persistence` | `backend/include/persistence/`  | `Database`, `UserRepository`, `PlayerProfileRepository`, `LeaderboardRepository`, wallet, store, and inventory repositories |
| `common`      | `backend/include/common/`       | `Types`, `Enums`, `Results` |

Gameplay modules provide playable characters, enemies, physics, collision, combat, and Fighting/Running/Flappy mode logic. GameManager selects a mode and exposes a render-ready shared::GameState through GameAPI. The frontend still owns key polling, drawing, sound, and animation.

## Data Flow

```text
Frontend → GameAPI → Backend Systems → Persistence
```

Never `Frontend → SQLite` and never `Frontend → internal backend classes`.

## GameAPI

`shared/include/shared/GameAPI.h` is the only frontend boundary. It exposes controls, gameplay snapshots, store items, and cosmetic ownership/equipping without exposing SQLite or internal service classes.

## OOP Architecture

- **Encapsulation** — e.g. `Wallet` will keep its balance private behind
  `getBalance()` / `spend()` / `addFunds()`.
- **Inheritance**: `GameMode` is an abstract base with `start()`, `update(dt)`, `handleAction(action)`, and `finished()`; `FightingMode`, `RunningMode`, and `FlappyMode` derive from it.
- **Polymorphism** — the core holds a `std::unique_ptr<GameMode> currentMode`
  and dispatches through the base interface.
- **Abstraction** — services (`AuthService`, `PaymentService`,
  `PlayerProfileRepository`, `WalletRepository`) hide their implementation
  details behind interfaces.
- **Composition** — smaller systems are composed together rather than
  forced into unnecessary inheritance hierarchies.

## Dependency Boundaries

- `shared` is header-only and has no third-party dependencies.
- `backend` depends on `shared` and configured persistence/security libraries. It never depends on SFML.
- `frontend` depends on `shared` and SFML 3. It never accesses repositories or wallet internals.
- `tests` use Catch2 and run without an SFML window.

Cosmetic ownership lives in the store inventory. GameAPI returns catalog entries with ownership flags and accepts equip requests only for the built-in skin or purchased skins. GameState exposes the selected item ID; the frontend maps it to the catalog resourceKey. Cosmetics are optional and do not affect game rules.
