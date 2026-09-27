# Team

4 developers: 2 backend, 2 frontend.

## Backend Developer 1 — Game Systems

Owns:

```text
backend/include/core/       backend/src/core/
backend/include/entities/    backend/src/entities/
backend/include/gameplay/    backend/src/gameplay/
```

Responsibilities:

- `Game`, `GameState`, `GameManager`, `GameMode`
- `Player`, `Enemy`, `Obstacle`
- `FightingMode`, `RunningMode`, `FlappyMode`
- Movement, collision, combat, physics, scoring

## Backend Developer 2 — Services & Data

Owns:

```text
backend/include/auth/         backend/src/auth/
backend/include/player/        backend/src/player/
backend/include/payment/       backend/src/payment/
backend/include/persistence/   backend/src/persistence/
```

Responsibilities:

- `AuthService`, `Session`, `AliasService`
- `PlayerProfile`, `Inventory`
- `Wallet`, `PaymentService`, `Store`
- `Database`, repositories (User/Player/Wallet/Transaction)

## Frontend Developer 1 — Menu & UI

Potential ownership:

```text
frontend/src/menu/
frontend/src/auth-ui/
frontend/src/navigation/
```

Responsibilities:

- SFML window bootstrap
- Login/registration screens
- Main menu, navigation, UI components/buttons

## Frontend Developer 2 — Gameplay & Rendering

Potential ownership:

```text
frontend/src/gameplay/
frontend/src/rendering/
frontend/src/hud/
```

Responsibilities:

- Gameplay rendering and game-state visualization
- HUD, animations, visual feedback

## Shared Responsibilities

Both backend developers collaborate on `shared/` — common types, enums,
DTOs, and the `GameAPI` contract. Breaking changes to any shared interface
must be communicated to the rest of the team before merging (see
`docs/CONTRIBUTING.md`).

The exact ownership split above can be adjusted by the team as the project
evolves; this file reflects the intended default division of labor from
day one.
