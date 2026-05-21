# Commit Conventions

Follow the [Conventional Commits](https://www.conventionalcommits.org/) style:

- `feat:` new features
- `fix:` bug fixes
- `docs:` documentation changes
- `refactor:` code refactoring without behaviour changes
- `test:` when adding or modifying tests

Format: `type(scope): short description` where the scope is optional. Keep messages short and imperative.

## Git Trailers (OMC extended format)

Use git trailers to preserve decision context in every commit message when the change is non-trivial.

Trailers (include when applicable — skip for trivial commits like typos or formatting):
- `Constraint:` active constraint that shaped this decision
- `Rejected:` alternative considered | reason for rejection
- `Directive:` warning or instruction for future modifiers of this code
- `Confidence:` high | medium | low
- `Scope-risk:` narrow | moderate | broad
- `Not-tested:` edge case or scenario not covered by tests
