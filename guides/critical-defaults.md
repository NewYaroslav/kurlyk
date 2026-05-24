# Critical Defaults

Mandatory rules for every repository task.

## Before Editing

- Check `git status --short` before editing and do not overwrite user changes.
- Prefer codebase-memory MCP tools (`mcp__codebase-memory__*`) over raw grep for cross-file search and exploration.
- Keep edits scoped to the requested task and the relevant local style.

## Compatibility

- Preserve C++11 compatibility unless the change is explicitly C++17-only and properly guarded.
- Do not use lambda default captures (`[&]` or `[=]`) in C++ code. List every captured variable explicitly, and capture `this` explicitly when member access is needed.
- Do not introduce `thread_local` STL scratch buffers in serialization paths.

## Source of Truth

- Treat `include/` as the primary source of truth. Generated copies under build directories are not source files.

## Testing

- For code changes, verify with the narrowest relevant tests (at least one example from `examples/`), and test both C++11 and C++17 when the change touches shared headers or template behavior.

## Documentation

- Keep `README.md` and `README-RU.md` synchronized; when one changes, update the other in the same change unless the user explicitly narrows the scope.

## Git

See [Git workflow](git-workflow.md) for the full branch and PR policy.
