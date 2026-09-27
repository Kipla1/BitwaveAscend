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
| `persistence` | `backend/include/persistence/`  | `Database`, `UserRepository`, `PlayerRepository`, `WalletRepository`, `TransactionRepository` |
| `common`      | `backend/include/common/`       | `Types`, `Enums`, `Results` |

All headers above are currently empty/minimal placeholders (repository
initialization phase) — see the `TODO` comment at the top of each file for
which development day introduces its real content.

## Data Flow

```text
Frontend → GameAPI → Backend Systems → Persistence
```

Never `Frontend → SQLite` and never `Frontend → internal backend classes`.

## GameAPI

`shared/include/shared/GameAPI.h` is the single entry point the frontend
is allowed to call through. See [`API_CONTRACT.md`](API_CONTRACT.md) for
the full contract. It is a design contract at this stage — not every
method needs a real implementation until the day that introduces it (see
`DEVELOPMENT_PLAN.md`).

## OOP Architecture

- **Encapsulation** — e.g. `Wallet` will keep its balance private behind
  `getBalance()` / `spend()` / `addFunds()`.
- **Inheritance** — `GameMode` is an abstract base with `initialize()`,
  `update(dt)`, `handleAction(action)`, `isFinished()`; `FightingMode`,
  `RunningMode`, `FlappyMode` derive from it.
- **Polymorphism** — the core holds a `std::unique_ptr<GameMode> currentMode`
  and dispatches through the base interface.
- **Abstraction** — services (`AuthService`, `PaymentService`,
  `PlayerRepository`, `WalletRepository`) hide their implementation
  details behind interfaces.
- **Composition** — smaller systems are composed together rather than
  forced into unnecessary inheritance hierarchies.

## Dependency Boundaries

- `shared` — header-only, zero third-party dependencies. Both backend and
  frontend depend on it; it depends on neither.
- `backend` — depends only on `shared` at this stage. Future systems will
  add SQLite, libsodium, nlohmann/json, and spdlog — **never SFML**.
- `frontend` — depends on `shared` and SFML 3. Never depends on backend
  internals directly, only through `GameAPI`.
- `tests` — depends on `backend` and Catch2. Must never require an SFML
  window/context to run (see `DEVELOPMENT_PLAN.md`, "Testing Strategy").
