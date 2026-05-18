# Agent Instructions

`kurlyk` is a header-only C++11/17 library built on top of **libcurl** and
**Simple-WebSocket-Server** to provide asynchronous HTTP and WebSocket clients
with rate limiting, automatic reconnection, and proxy support.

## Reading Order

1. `guides/codebase-orientation.md` for the library model and layout.
2. `guides/rate-limiting-and-streaming.md` for rate-limiter architecture,
   partitioned keys, sequential mode, and streaming callbacks.
3. The task-specific playbook for build, C++, headers, dependencies, or commits.

## Project Rules

- Keep the public library under `include/kurlyk/`.
- Keep examples in `examples/` and tests in `tests/`.
- Preserve the library as dependency-light C++ unless a task explicitly asks for
  integration with another package.
- Prefer clear ownership, value-type request/response contracts, and RAII handle
  ownership.
- Use Doxygen comments for public APIs and non-obvious contracts.
- Use Conventional Commits for commit messages.

## Before Editing

Match the task to the guide and read it before changing code:

| Task area | Read first |
| --- | --- |
| HTTP request manager, rate limiter, retry, cancellation, streaming | `guides/concurrency.md` + `guides/rate-limiting-and-streaming.md` |
| CMake, build, integration tests | `guides/build-and-test.md` |
| Public API, headers, naming, Doxygen style | `guides/codebase-orientation.md` + `guides/cpp-development-guidelines.md` |
| Commit messages | `guides/commit-conventions.md` |

## Critical Defaults

- Check `git status --short` before editing and do not overwrite user changes.
- Prefer `rg` / `rg --files` for repository search; avoid broad `find` or `ls -R`.
- Keep edits scoped to the requested task and the relevant local style.
- Keep `README.md` and `README-RU.md` synchronized; when one changes, update
  the other in the same change unless the user explicitly narrows the scope.
- Preserve C++11 compatibility unless the change is explicitly C++17-only and
  properly guarded.
- When modifying library headers, compile at least one example or run the
  narrowest relevant tests. Verify with both C++11 and C++17 when the change
  touches shared headers or template behavior.
- Do not use lambda default captures (`[&]` or `[=]`) in C++ code. List every
  captured variable explicitly, and capture `this` explicitly when member access
  is needed.
- Do not introduce `thread_local` STL scratch buffers in serialization paths.

