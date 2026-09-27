# Bitwave Ascend

A 2D game built for a university Object-Oriented Programming group project,
demonstrating a full user lifecycle: registration/login, multiple game
modes, persistent progression, and a virtual wallet/store.

> **Status:** Repository initialization only. No gameplay, authentication,
> payments, or persistence logic has been implemented yet. See
> [Scope Rules](#scope-rules) below.

## Description

Bitwave Ascend is a 2D game with three initial modes — Fighting, Endless
Runner, and Gravity/Flappy — sharing one account system, one persistent
save system, and one in-game economy (wallet + store + inventory). The
project exists to demonstrate encapsulation, inheritance, polymorphism,
abstraction, composition, separation of concerns, persistence, modularity,
scalability, basic security, and performance-conscious design.

## Goals

- Ship one complete, playable vertical slice (login → menu → gameplay →
  score → game over → save) by Day 7.
- Keep the C++ application core (`backend/`) fully independent of SFML.
- Keep the frontend (`frontend/`) ignorant of SQLite, wallet internals,
  authentication internals, and raw backend classes — it only talks to
  `GameAPI`.
- Finish all three game modes, wallet/store, and full persistence within
  the 14-day timeline (see `docs/DEVELOPMENT_PLAN.md`).

## Technology Stack

| Concern            | Choice                     |
|---------------------|----------------------------|
| Language            | C++20                      |
| Build system        | CMake (+ CMakePresets)     |
| Rendering/Input     | SFML 3                     |
| Database            | SQLite                     |
| JSON                | nlohmann/json              |
| Password hashing    | libsodium (Argon2id)       |
| Testing             | Catch2                     |
| Logging             | spdlog                     |
| Package management  | vcpkg                      |
| Version control     | Git / GitHub               |

## Architecture

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

`backend/` determines **what** exists and **what** happens. `frontend/`
determines **how** it looks and **how** the user interacts with it. Full
details: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Repository Structure

```text
BitwaveAscend/
├── backend/            # C++ application core (no SFML dependency)
│   ├── include/        # core, entities, gameplay, auth, player, payment, persistence, common
│   ├── src/
│   ├── tests/
│   └── data/           # local SQLite files (git-ignored)
├── frontend/           # SFML rendering / input / UI / audio
├── shared/             # frontend<->backend contract (GameAPI, DTOs, enums)
├── docs/               # architecture, API contract, DB schema, plan, security, contributing
├── tests/              # Catch2 test suite (backend logic, no SFML window required)
├── CMakeLists.txt
├── CMakePresets.json
├── vcpkg.json
├── TEAM.md
└── LICENSE
```

## Build Instructions

Requirements: a C++20 compiler, CMake ≥ 3.21, and (for the full build)
[vcpkg](https://github.com/microsoft/vcpkg).

**Backend + shared only (no external dependencies required):**

```bash
cmake --preset default
cmake --build --preset default
```

**Full build (frontend + tests, requires vcpkg):**

```bash
export VCPKG_ROOT=/path/to/vcpkg
cmake --preset full
cmake --build --preset full
ctest --preset full
```

If SFML or Catch2 are not found, CMake prints a warning and skips that
target rather than failing the whole configure — the backend always
configures and builds.

## Development

- Two-week timeline and daily milestones: [`docs/DEVELOPMENT_PLAN.md`](docs/DEVELOPMENT_PLAN.md)
- Architecture and module boundaries: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)
- Frontend/backend contract: [`docs/API_CONTRACT.md`](docs/API_CONTRACT.md)
- Planned database schema: [`docs/DATABASE.md`](docs/DATABASE.md)
- Security requirements: [`docs/SECURITY.md`](docs/SECURITY.md)

## Team Responsibilities

See [`TEAM.md`](TEAM.md) for the full breakdown. Summary:

- **Backend Developer 1** — Game systems: `core/`, `entities/`, `gameplay/`
- **Backend Developer 2** — Services & data: `auth/`, `player/`, `payment/`, `persistence/`
- **Frontend Developer 1** — Menu & UI
- **Frontend Developer 2** — Gameplay & rendering

## Timeline

14-day schedule from architecture setup (Day 1) to finalization (Day 14),
with a critical playable-vertical-slice milestone on Day 7. Full breakdown
in [`docs/DEVELOPMENT_PLAN.md`](docs/DEVELOPMENT_PLAN.md).

## Contribution Guidelines

- Work on feature branches (`feature/backend-*`, `feature/frontend-*`);
  keep `main` buildable.
- Commit messages follow Conventional Commits (`feat:`, `fix:`, `docs:`,
  `test:`, `chore:`).
- Changes to `shared/` (the GameAPI/DTO contract) must be flagged to the
  whole team before merging.
- Full details: [`docs/CONTRIBUTING.md`](docs/CONTRIBUTING.md).

## Scope Rules

This is a two-week university project — avoid scope creep. Do **not**
introduce Django, Flask, a Node.js backend, REST APIs, PostgreSQL, MySQL,
Redis, Docker, WebSockets, cloud infrastructure, or microservices unless a
future requirement explicitly demands it. The current phase is
**repository initialization only**: no gameplay, authentication, payment,
or persistence logic exists yet — see the `TODO` comments throughout
`backend/include/` for what's intentionally left unimplemented.
