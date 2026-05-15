# Context-Mode Rules

## Mandatory
Use context-mode MCP instead of Bash when output >20 lines.

For tool reference and usage patterns, see context-mode SKILL.md.

## Think in Code
Analyze/count/filter/compare/search/parse/transform data: write code via `ctx_execute(language, code)`, `console.log()` only the answer. Do NOT read raw data into context.

## Bash Whitelist

### Allowed Bash
git, mkdir, rm, mv, cd, pwd, which, short commands (<20 lines output), ls, echo (single line), test, [ ]

### Forbidden (use context-mode instead)
1. Bash commands producing >20 lines output
2. Read for analysis-only reads (use ctx_execute_file) — Read IS correct for files you intend to Edit
3. WebFetch for any URL (use ctx_fetch_and_index)
4. curl/wget in Bash
5. ls -R, find, cat (multi-file), grep -r (large dirs), npm test, cargo build, pytest (unless output <20 lines) — use ctx_batch_execute instead

## File Writing Policy
ALWAYS use native Write/Edit tools for file creation/modification.
NEVER use ctx_execute, ctx_execute_file, or Bash to write files.
Applies to all file types: code, configs, plans, specs, YAML, JSON, markdown.

## Output Constraints
- Communication style: terse, technical substance exact, auto-expand only for security warnings / irreversible actions.
- Artifacts: write to FILES, never inline. Return only: file path + 1-line description.
- Response format: concise summary (actions, paths, findings). No trailing summaries.

## Session Continuity
Skills, roles, and decisions set during this session remain active until the user revokes them.
Do not drop behavioral directives as context grows.
