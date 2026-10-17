# Database (Planned Schema)

SQLite, introduced Day 3. **Not implemented during repository
initialization** — this document exists so persistence work on Day 3+ has
an agreed target instead of being designed ad hoc.

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

## `transactions`

| Column      | Notes |
|-------------|-------|
| `id`        | Primary key |
| `user_id`   | FK → `users.id` |
| `item_id`   | FK → `store_items.id` |
| `amount`    | |
| `timestamp` | |
| `status`    | e.g. pending/succeeded/failed |

## `payment_orders`
{
  id            integer [primary key]
  user_id       integer [not null, ref: > users.id]
  reference     text [unique, not null]
  amount_cents  integer [not null]
  currency      text [not null]
  coins         integer [not null]
  status        text [not null, default: 'pending', note: 'pending | succeeded | failed']
  created_at    timestamp [not null, default: `CURRENT_TIMESTAMP`]
}
## Access Rules

- Only `backend/persistence/` repositories touch SQLite directly.
- All queries are parameterized (no string-concatenated SQL).
- The frontend never accesses SQLite; it only ever sees `PlayerProfile`,
  `GameState`, `StoreItem`, wallet balance, etc. through `GameAPI`.

