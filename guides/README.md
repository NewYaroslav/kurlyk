# guides/

Shared AI-agent instruction files for `kurlyk`. Both Claude Code and Codex
access these playbooks through `AGENTS.md`.

## Reading Order

1. `codebase-orientation.md` for the library model, public API, and source layout.
2. `rate-limiting-and-streaming.md` for rate-limiter architecture, partitioned
   limits, streaming callbacks, and backpressure.
3. The task-specific playbook for build, C++, headers, dependencies, or commits.

## Files

- `codebase-orientation.md` — project map, subsystem model, extension recipes,
  safety invariants, and utility inventory.
- `rate-limiting-and-streaming.md` — `HttpRateLimiter`, `HttpRateLimitHandle`,
  `HttpBatchRequestHandler`, streaming callbacks, and backpressure design.
- `dependencies.md` — dependency policy for libcurl, OpenSSL, Asio,
  Simple-WebSocket-Server, and optional JSON.
- `build-and-test.md` — CMake configure/build/test guidance.
- `cpp-development-guidelines.md` — C++ style, naming, comments, and API
  boundaries.
- `header-implementation-guidelines.md` — ownership rules for `.hpp`, `.ipp`, and
  `.tpp` files.
- `commit-conventions.md` — Conventional Commit format and grouping rules.

## Maintenance

- Keep each file focused on one concern.
- Update the canonical topic file instead of copying rules into `AGENTS.md`.
- Do not add tool-specific rules unless they are genuinely different for that
  tool.
