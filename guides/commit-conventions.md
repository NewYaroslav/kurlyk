# Commit Conventions

Use [Conventional Commits](https://www.conventionalcommits.org/).

## Format

```text
type(scope): imperative summary

Explain why the change is needed and any important trade-offs.
```

## Header

- Keep the first line at 50 characters or less when practical.
- Use imperative mood.
- Use a conventional prefix such as `feat:`, `fix:`, `refactor:`, `docs:`,
  `test:`, `build:`, or `chore:`.
- Add a scope when it clarifies the touched area (e.g., `http:`, `websocket:`,
  `core:`, `docs:`).

## Body

- Explain why the change exists, not only what files changed.
- Mention compatibility, safety, or migration notes when relevant.
- Do not include real secrets, access keys, private tokens, local credentials,
  or machine-specific paths that should remain private.

## Change Grouping

- Keep unrelated changes in separate commits.
- Do not mix dependency updates with feature work unless the dependency change is
  required for that feature.
- Do not include edits inside generated build directories unless the task
  explicitly asks for them.
