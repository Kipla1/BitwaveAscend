# Security

Requirements the backend must satisfy once the relevant systems are
implemented (Day 3+). Not implemented yet — repository initialization
only declares the requirements.

## Password Hashing

- Never store plaintext passwords.
- Hash passwords with libsodium's Argon2id (`crypto_pwhash_str` /
  `crypto_pwhash_str_verify`), not a custom scheme.
- Password hashes never cross the `GameAPI` boundary to the frontend.

## Data Encapsulation

- `Wallet`, `PlayerProfile`, and similar classes keep sensitive state
  private, exposed only through explicit accessor/mutator methods that can
  enforce invariants (e.g. `Wallet::spend()` refusing to go negative).
- The frontend only ever sees the public-facing DTOs defined in
  `shared/include/shared/Contracts.h` (`PlayerProfile`, `GameState`,
  `StoreItem`, ...) — never internal backend classes.

## Wallet Protection

- Frontend must not directly modify wallet balances; all changes go
  through `PaymentService`/`Wallet` on the backend.
- Purchases validate: item existence, current price, sufficient wallet
  balance, and that the requesting user/session is authorized — before any
  balance or inventory change is committed.

## SQL Parameterization

- All SQLite queries use parameterized statements (bound parameters), no
  string-concatenated SQL, to prevent SQL injection.
- Only `backend/persistence/` repositories touch SQLite; nothing else in
  the codebase issues SQL directly.

## Sensitive Data Handling

- Never log passwords, password hashes, or other sensitive credentials
  (see `spdlog` usage guidelines once logging is introduced).
- Local SQLite database files under `backend/data/` are git-ignored so a
  developer's real save data/credentials are never committed.
