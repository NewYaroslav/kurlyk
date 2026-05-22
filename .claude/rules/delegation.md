# Agent Delegation

## Model Param — Mandatory
EVERY Agent tool call MUST include `model` param:
- haiku: search, exploration, simple lookups
- sonnet: standard implementation, verification
- opus: architecture, refactoring, complex debugging

Missing `model` param = error, not warning. Add it before submitting.

## Full Agent Catalog

### haiku (lightweight)

| Agent | Task type |
|-------|-----------|
| explore | File search, codebase navigation, pattern discovery |
| writer | Documentation, comments, README, changelog |

### sonnet (standard)

| Agent | Task type |
|-------|-----------|
| debugger | Root-cause tracing, error diagnosis, breakpoint analysis |
| executor | Code changes, implementation, file edits |
| verifier | Test verification, build checks, outcome validation |
| tracer | Evidence-driven tracing, causal chains, hypothesis ranking |
| security-reviewer | Vulnerability assessment, attack surface analysis, CVE review |
| test-engineer | TDD, test coverage, test design, fuzzing |
| designer | UI/UX design, component architecture, design systems |
| qa-tester | Manual QA scenarios, acceptance testing, edge-case exploration |
| scientist | Data analysis, experiments, benchmarks, statistical validation |
| document-specialist | SDK/API docs lookup, library research, Context7 queries |
| git-master | Git operations, branching strategy, merge conflict resolution |

### opus (heavyweight)

| Agent | Task type |
|-------|-----------|
| analyst | Deep analysis, data interpretation, root-cause hypothesis |
| planner | Task decomposition, roadmap, estimation |
| architect | Architecture decisions, system design, refactoring strategy |
| code-reviewer | Code quality, pattern compliance, maintainability review |
| code-simplifier | Code reduction, dead code elimination, abstraction cleanup |
| critic | Antithesis generation, design challenge, assumption questioning |

## Routing

| Task type | Agent | Model |
|-----------|-------|-------|
| Deep analysis | analyst | opus |
| Planning | planner | opus |
| Architecture | architect | opus |
| Critique | critic | opus |
| Code changes | executor | sonnet |
| Complex code changes | executor | opus |
| Code simplification | code-simplifier | opus |
| Code review | code-reviewer | opus |
| Verification | verifier | sonnet |
| Security review | security-reviewer | sonnet |
| Exploration | explore | haiku |
| SDK/docs lookup | document-specialist | sonnet |
| Root-cause debugging | debugger | sonnet |
| Evidence tracing | tracer | sonnet |
| Data/research | scientist | sonnet |
| Test design | test-engineer | sonnet |
| QA testing | qa-tester | sonnet |
| Design | designer | sonnet |
| Writing | writer | haiku |
| Git ops | git-master | sonnet |

TaskCreate = conversation tracking only, NOT delegation.

## MCP Server Routing

| Category | Agents | Primary | Fallback |
|----------|--------|---------|----------|
| Analysis / Architecture | explore, analyst, tracer, architect, scientist | Codebase Memory (`mcp__codebase-memory__*`), ctx_*, python_repl, Grep/Glob, session_search | Bash, DDG |
| Implementation | executor, verifier, debugger, test-engineer | Edit/Write, LSP, ast_grep, ctx_execute, Bash (tests) | Bash, Grep, python_repl |
| Review & Security | code-reviewer, security-reviewer | LSP, ast_grep_search, Grep, ctx_execute_file | Read, Bash |
| Specialist | document-specialist, architect, writer, git-master | context7, GitHub, LSP, ctx_execute_file, Read | DDG, Fetch, gh CLI |

## Skill Routing

### Execution & Planning

| Skill | Primary Agent | Supporting Agents |
|-------|--------------|-------------------|
| autopilot | executor | planner, verifier, code-reviewer |
| ralph | executor | verifier, debugger |
| ultrawork | executor (multiple) | planner, verifier |
| team | planner (team-plan) | executor (team-exec), verifier (team-verify) |
| ccg | executor | critic (codex), code-reviewer (gemini) |
| omc-plan | planner | architect, critic |
| ralplan | planner | architect, critic, executor |
| deep-interview | planner | critic |
| deepinit | architect | explorer, writer |
| sciomc | scientist | analyst, verifier |
| trace | tracer | debugger, analyst |

### Quality & Utilities

| Skill | Primary Agent | Supporting Agents |
|-------|--------------|-------------------|
| ultraqa | qa-tester | verifier, debugger |
| verify | verifier | debugger |
| ai-slop-cleaner | code-simplifier | code-reviewer, verifier |
| self-improve | code-simplifier | architect, critic |
| external-context | document-specialist | analyst |
| release | git-master | verifier |
| writer-memory | writer | — |
| cancel | (orchestrator direct) | — |
| omc-doctor | debugger | — |

## Routing Laws
See [software-laws.md](software-laws.md) for full definitions.
- [Conway's Law](software-laws.md#conways-law) — role boundaries
- [Tesler's Law](software-laws.md#teslers-law) — complexity conservation
- [Law of Demeter](software-laws.md#law-of-demeter) — agent→orchestrator→agent only
- [Postel's Law](software-laws.md#postels-law) — strict output, tolerant input
- [Brooks's Law](software-laws.md#brooks-law) — decompose, don't clone
- [Dunbar's Number](software-laws.md#dunbars-number) — max 5 coordinated agents
