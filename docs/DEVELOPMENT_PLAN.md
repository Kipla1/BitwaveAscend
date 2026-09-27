# Development Plan (14 Days)

## Day 1 — Architecture & Environment *(this phase)*

- Initialize Git repository
- Initialize CMake, configure C++20
- Configure dependency management (vcpkg manifest)
- Establish `backend/` / `frontend/` / `shared/` directory structure and
  separation
- Establish documentation (this file, architecture, API contract,
  database, security, contributing)
- Establish testing structure (Catch2, not yet building real tests)
- Confirm everyone can clone and build

**Definition of done for Day 1:** see the repository-initialization
checklist in the root `README.md` header / PR description — repository
configures and builds; no gameplay/auth/payment/persistence logic exists.

## Day 2 — Core Interfaces

- Backend: `Game`, `GameState`, `GameManager`, `GameMode`, `Entity`,
  `PlayerProfile`, `Wallet`, `AuthService` interfaces
- Shared: `GameModeType`, `PlayerAction`, `GameState`, API contract frozen
  after integration discussion
- Frontend: basic SFML project structure

## Day 3 — Authentication + Database

SQLite, registration, login, logout, sessions, password hashing (Argon2id
via libsodium), basic user storage. Begin entity system.

## Day 4 — Profile + Persistence

`AliasService`, `PlayerProfile`, repositories, save/load, `GameState`,
basic movement, health, score.

## Day 5 — Game Mode Architecture

`GameMode` interface, `FightingMode`/`RunningMode`/`FlappyMode` skeletons,
`GameAPI`, session integration.

## Day 6 — Fighting Mode

Player, enemy, attack, damage, health, death, score, collision.

## Day 7 — First Integration (critical milestone)

```text
Login → Main Menu → Fighting Mode → Gameplay → Score → Game Over → Save
```

One complete playable vertical slice must exist.

## Day 8 — Endless Runner

Movement, jump, gravity, obstacles, collision, scrolling, score, game
over.

## Day 9 — Flappy Mode

Gravity, flap, pipes/obstacles, collision, score, game over. All three
modes work at the logic level.

## Day 10 — Wallet + Store

Wallet, virtual currency, store, inventory, `PaymentService`,
transactions, insufficient-funds handling.

## Day 11 — Full Persistence

Ensure these survive an application restart: user, alias, score,
progress, wallet, inventory, transactions.

## Day 12 — Full Frontend Integration

```text
Login → Main Menu → Select Mode → Play → Game Over → Save → Store → Purchase → Inventory
```

## Day 13 — Testing + Bug Fixing

Registration, login, wrong password, duplicate username, gameplay,
collision, game over, wallet, insufficient funds, purchases, persistence,
edge cases.

## Day 14 — Finalization

Integration fixes, refactoring, documentation, UML/class diagrams, code
cleanup, demo preparation, final build verification.

## Critical Scope Rule

Two-week university project — avoid scope creep. Do not introduce Django,
Flask, a Node backend, REST APIs, PostgreSQL, MySQL, Redis, Docker,
WebSockets, cloud infrastructure, microservices, or Box2D unless
explicitly required later. Simple working systems beat sophisticated
unfinished ones.

If the project falls behind, prioritize in this order:

1. Buildability
2. Authentication
3. One complete game mode
4. Persistence
5. Remaining game modes
6. Store/payment
7. Polish

## Testing Strategy

Catch2, backend logic testable without an SFML window:

```text
tests/
├── test_auth.cpp
├── test_wallet.cpp
├── test_game_modes.cpp
├── test_collision.cpp
└── test_persistence.cpp
```

E.g. collision logic tested without an SFML window; wallet logic tested
without the frontend; authentication and persistence tested
independently.
