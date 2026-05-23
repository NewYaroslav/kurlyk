# Git Workflow

## Branch Policy

All code changes must go through a feature branch and a pull request.
Never commit directly to `main`.

## Required Steps Before Creating a Branch

1. Check the current state:
   ```bash
   git status
   git branch -vv
   ```
2. Update `main` to the latest remote state:
   ```bash
   git fetch origin
   git checkout main
   git pull origin main
   ```
3. Only after `main` is up-to-date, create the feature branch:
   ```bash
   git checkout -b <prefix>/<short-description>
   ```

## Branch Naming

| Prefix | Use for |
| --- | --- |
| `feat/` | New functionality |
| `fix/` | Bug fixes |
| `refactor/` | Code restructuring without behavior changes |
| `test/` | Adding or modifying tests only |
| `docs/` | Documentation changes |
| `chore/` | Build, CI, dependency, or maintenance tasks |
| `perf/` | Performance improvements |

Examples:
- `feat/otlp-gzip-compression`
- `fix/payload-splitter-overflow`
- `test/backpressure-race`

## Workflow

1. Create a focused branch from up-to-date `main`.
2. Make atomic commits with conventional messages (see `commits.md`).
3. Push the branch to origin:
   ```bash
   git push -u origin <branch-name>
   ```
4. Open a pull request on GitHub.
5. Merge only after review and passing CI.

## Agent Rule

When starting any task (feature, bug fix, improvement, documentation), an AI agent must:

1. Do not edit or commit while on `main`. If currently on `main`, create a branch before editing files.
2. Verify `main` is current with `origin/main`.
3. Create a feature branch with an appropriate prefix.
4. Do all work on that branch.
5. Push the branch and instruct the user to open a PR instead of merging directly.

No direct commits to `main`, including trivial documentation fixes, unless the repository owner explicitly overrides this rule for that exact change.
