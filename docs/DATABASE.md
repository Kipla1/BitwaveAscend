# Database

SQLite persists accounts, player progress, wallets, inventory, store items,
and purchase history. The leaderboard uses saved scores and aliases without
duplicating those records.

## `users`

| Column          | Notes |
|-----------------|-------|
| `id`            | Primary key |
| `username`      | Unique |
| `password_hash` | Argon2id via libsodium — never plaintext, never logged |
| `alias`         | Unique in-game display name |
| `created_at`    | Timestamp |

## `player_progress`

| Column          | Notes |
|-----------------|-------|
| `user_id`       | FK → `users.id` |
| `highest_score` | |
| `current_level` | |
| `equipped_skin_id` | Defaults to `0` (built-in skin) |

The leaderboard joins saved highest scores to `users.alias`, sorts scores
descending, and uses user ID for stable ties. Zero-score accounts are omitted.

## `wallets`

| Column     | Notes |
|------------|-------|
| `user_id`  | FK → `users.id` |
| `balance`  | In-game currency, integer (smallest unit) — never floating point |

## `inventory`

| Column     | Notes |
|------------|-------|
| `user_id`  | FK → `users.id` |
| `item_id`  | FK → `store_items.id` |
| `quantity` | |

## `store_items`

| Column        | Notes |
|---------------|-------|
| `id`          | Primary key |
| `name`        | |
| `price`       | |
| `description` | |
| `kind` | `power_up` or `cosmetic`; defaults to `power_up` for older saves |
| `resource_key` | Frontend asset lookup key for a cosmetic |

## `transactions`

| Column      | Notes |
|-------------|-------|
| `id`        | Primary key |
| `user_id`   | FK → `users.id` |
| `item_id`   | FK → `store_items.id` |
| `amount`    | |
| `timestamp` | |
| `status`    | e.g. pending/succeeded/failed |

## Access Rules

- Only repositories under `backend/src/persistence/` touch SQLite directly.
- All queries are parameterized (no string-concatenated SQL).
- The frontend never accesses SQLite; it only ever sees `PlayerProfile`,
  `GameState`, `StoreItem`, wallet balance, etc. through `GameAPI`.

Firebase cloud sync is not connected yet. Local SQLite saves progress during
play when a new personal best is reached; remote sync needs Firebase Auth,
Firestore, and an authenticated client.

