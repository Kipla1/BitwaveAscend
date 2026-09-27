# Contributing

## Branch Naming

Work on feature branches; keep `main` buildable at all times.

```text
feature/backend-game-core
feature/backend-auth
feature/backend-payment
feature/backend-persistence
feature/frontend-menu
feature/frontend-gameplay
feature/frontend-ui
```

## Commit Conventions

Conventional Commits style:

```text
feat: initialize CMake project
feat: add shared game contracts
feat: add GameMode interface
docs: define backend architecture
test: add wallet tests
fix: resolve database connection issue
```

Avoid non-descriptive messages: `stuff`, `changes`, `final`, `fixed`,
`update`, `asdf`.

Group commits logically (by feature/concern) rather than one commit per
file.

## Pull Requests

- Open a PR from your feature branch into `main`.
- PR description should say what changed and why, and call out any change
  to `shared/` explicitly.
- At least one other team member should look over changes that touch
  `shared/` before merging, since both frontend and backend depend on it.

## Code Style

- `namespace bitwave { ... }` (with sub-namespaces per module, e.g.
  `bitwave::core`, `bitwave::payment`) — avoid polluting the global
  namespace.
- Small classes, single responsibility, RAII, smart pointers, const
  correctness.
- No unnecessary global mutable state, no unnecessary inheritance, prefer
  composition where it fits.
- No giant classes/functions, no magic numbers, no duplicated logic, no
  circular dependencies.

## Shared Interface Changes

Any change to `shared/include/shared/Contracts.h` or `GameAPI.h` affects
both backend developers and both frontend developers. Before merging such
a change:

1. Post in the team channel describing the change and why it's needed.
2. Confirm nobody has in-flight work that conflicts with it.
3. Update `docs/API_CONTRACT.md` in the same PR.
