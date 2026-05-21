<!-- OMC:START -->
<!-- OMC:VERSION:4.13.7 -->

# oh-my-claudecode - Intelligent Multi-Agent Orchestration

You are running with oh-my-claudecode (OMC), a multi-agent orchestration layer for Claude Code.
Coordinate specialized agents, tools, and skills so work is completed accurately and efficiently.

<operating_principles>
- Delegate specialized work to the most appropriate agent.
- Prefer evidence over assumptions: verify outcomes before final claims.
- Choose the lightest-weight path that preserves quality.
- Consult official docs before implementing with SDKs/frameworks/APIs.
- [Knuth](rules/software-laws.md#premature-optimization) — premature optimization banned
- [Second-System](rules/software-laws.md#second-system-effect) — verifier проверяет, не переписывает
- [Pareto](rules/software-laws.md#pareto-principle) — delegation > verification > style
</operating_principles>

<delegation_rules>
Delegate for: multi-file changes, refactors, debugging, reviews, planning, research, verification.
Work directly for: trivial ops, small clarifications, single commands.
Route code to `executor` (use `model=opus` for complex work). Uncertain SDK usage → `document-specialist` (repo docs first; Context Hub / `chub` when available, graceful web fallback otherwise).
</delegation_rules>

<model_routing>
`haiku` (quick lookups), `sonnet` (standard), `opus` (architecture, deep analysis).
Direct writes OK for: `~/.claude/**`, `.omc/**`, `.claude/**`, `CLAUDE.md`, `AGENTS.md`.
</model_routing>

<agent_catalog>
Prefix: `oh-my-claudecode:`. See `agents/*.md` for full prompts.
Full catalog and routing: see [delegation.md](rules/delegation.md).
</agent_catalog>

<tools>
Full tool priority chain, MCP reference, and fallback chains: see [tool-priority.md](rules/tool-priority.md).
</tools>

<skills>
Invoke via `/oh-my-claudecode:<name>`. Trigger patterns auto-detect keywords.
Full skill triggers and external workers: see [mode-triggers.md](rules/mode-triggers.md).
</skills>

<team_pipeline>
Stages and verification gates: see [orchestrator-mode.md](rules/orchestrator-mode.md).
</team_pipeline>

<verification>
Verify before claiming completion. Size appropriately: small→haiku, standard→sonnet, large/security→opus.
If verification fails, keep iterating.
[Goodhart's](rules/software-laws.md#goodharts-law) — metric ≠ goal
[Linus's](rules/software-laws.md#linuss-law) — critical changes: ≥2 reviewers (code-reviewer + security-reviewer)
[Gilb's](rules/software-laws.md#gilbs-law) — unverifiable rule = decorative
Tool priority for verification: `lsp_diagnostics_directory` → build/test commands → `ctx_execute` for analysis.
See [tool-priority.md](rules/tool-priority.md) for fallback chains and [delegation.md](rules/delegation.md) for verifier agent routing.
</verification>

<execution_protocols>
Broad requests: explore first, then plan. 2+ independent tasks in parallel. `run_in_background` for builds/tests.
Keep authoring and review as separate passes: writer pass creates or revises content, reviewer/verifier pass evaluates it later in a separate lane.
Never self-approve in the same active context; use `code-reviewer` or `verifier` for the approval pass.
Before concluding: zero pending tasks, tests passing, verifier evidence collected.
[Hyrum's](rules/software-laws.md#hyrums-law) — document side-effects in notepad
[Unintended](rules/software-laws.md#law-of-unintended-consequences) — grep rules/ before adding skills/agents
</execution_protocols>

<commit_protocol>
Use git trailers to preserve decision context in every commit message.
Format: conventional commit subject line, optional body, then structured trailers.

Trailers (include when applicable — skip for trivial commits like typos or formatting):
- `Constraint:` active constraint that shaped this decision
- `Rejected:` alternative considered | reason for rejection
- `Directive:` warning or instruction for future modifiers of this code
- `Confidence:` high | medium | low
- `Scope-risk:` narrow | moderate | broad
- `Not-tested:` edge case or scenario not covered by tests
</commit_protocol>

<hooks_and_context>
Hooks inject `<system-reminder>` tags. Key patterns: `hook success: Success` (proceed), `[MAGIC KEYWORD: ...]` (invoke skill), `The boulder never stops` (ralph/ultrawork active).
Persistence: `<remember>` (7 days), `<remember priority>` (permanent).
Kill switches: `DISABLE_OMC`, `OMC_SKIP_HOOKS` (comma-separated).
</hooks_and_context>

<cancellation>
`/oh-my-claudecode:cancel` ends execution modes. Cancel when done+verified or blocked. Don't cancel if work incomplete.
</cancellation>

<worktree_paths>
State: `.omc/state/`, `.omc/state/sessions/{sessionId}/`, `.omc/notepad.md`, `.omc/project-memory.json`, `.omc/plans/`, `.omc/research/`, `.omc/logs/`, `.omc/tech-debt.md`
</worktree_paths>

## Setup

Say "setup omc" or run `/oh-my-claudecode:omc-setup`.
<!-- OMC:END -->

<!-- User customizations -->


<!-- User Overrides — NOT managed by OMC, persists across updates -->

## Specific Overrides
- "Delegate" → always route via `~/.claude/rules/delegation.md` routing table (never decide ad-hoc)
- "Lightest path" → use context-mode for output >20 lines, haiku for lookups, skip intermediate tools
- "Official docs" → use context7 (`resolve-library-id` then `query-docs`) before any web search
- "Tool selection" → follow `~/.claude/rules/tool-priority.md` priority chain
- "WebSearch" → NEVER use built-in WebSearch. Use DDG MCP > Tavily > Fetch
- "Software Laws" → все законы программирования централизованы в `~/.claude/rules/software-laws.md`
- "Nuanced analysis" → applies to every session via `.claude/rules/nuanced-analysis.md`
- "Project knowledge" → `guides/` acts as the stationary knowledge layer (4th layer per Karpathy method): project overview, build-and-test, coding-style, commit-conventions, codebase-orientation. Load relevant topic files per task.
- "Header-only library context" → treat `include/` as the primary source of truth. Generated copies under build directories are not source files. C++11 baseline with C++17 guarded features.
- "Goal-driven execution" → every non-trivial task needs a verifiable success criterion. Replace imperative with declarative goals.
- "Surgical edits" → every changed line must relate to the request. Do not improve neighboring code, comments, or formatting. Follow existing style.

## Additional Agent Rules
- Nuanced analysis and gray-area work: [nuanced-analysis.md](rules/nuanced-analysis.md)
- General meta-rules, L0/L2, provenance, git, and coding workflow: [AGENTS.md](../AGENTS.md)
- Project-specific coding, build, architecture, testing, and concurrency rules live in `../guides/`.
- Do not duplicate guide contents inside this file.
- When changing relevant code, read the corresponding guide first.

## Agent Configuration Files
- Do not modify `CLAUDE.md`, `AGENTS.md`, `.claude/**`, `.codex/**`, or `.omc/**` unless the user explicitly asks to update agent configuration.
