# Tool Priority Chain

## Web Search (non-Anthropic provider override)

Native WebSearch does NOT work with non-Anthropic providers (Fireworks, OpenRouter, etc.).
Use MCP search tools instead. Priority:
1. DDG Search MCP (`mcp__ddg-search__search`, `mcp__ddg-search__fetch_content`) — no API key needed
2. Tavily MCP (`mcp__tavily__tavily_search`, `mcp__tavily__tavily_extract`, `mcp__tavily__tavily_research`, `mcp__tavily__tavily_crawl`, `mcp__tavily__tavily_map`) — requires API key, has free tier
3. Fetch MCP (`mcp__fetch__fetch_markdown`, `mcp__fetch__fetch_txt`, `mcp__fetch__fetch_html`, `mcp__fetch__fetch_json`) — for known URLs only
NEVER use built-in WebSearch tool — it will fail with non-Anthropic providers.

## Mandatory Overrides (always applies first)

- WebSearch: NEVER use built-in tool — always MCP (DDG > Tavily > Fetch).
- Context-mode: mandatory for expected output >20 lines or bulk analysis. It handles large-output execution, but does not replace Codebase Memory discovery.
- Codebase Memory preflight: for non-trivial code tasks involving unknown structure, cross-file changes, architecture, refactoring, call chains, ownership, or "where is X implemented?", use Codebase Memory before Grep/Read/LSP.

## General Ordering (use first match)

### Local & Code Intelligence

1. Context7 (`mcp__plugin_context7_context7__resolve-library-id`, `mcp__plugin_context7_context7__query-docs`) — SDK/API docs before web search
2. Codebase Memory (`mcp__codebase-memory__search_graph`, `mcp__codebase-memory__trace_path`, `mcp__codebase-memory__get_code_snippet`, `mcp__codebase-memory__search_code`) — first-pass codebase discovery for non-trivial code tasks, cross-file relationships, call chains, architecture, ownership, and unknown implementation locations.
3. LSP (`lsp_hover`, `lsp_goto_definition`, `lsp_find_references`, `lsp_diagnostics`, `lsp_code_actions`, `lsp_code_action_resolve`, `lsp_document_symbols`, `lsp_workspace_symbols`, `lsp_prepare_rename`, `lsp_rename`, `lsp_servers`) — exact symbol lookup, definitions, references, diagnostics.
4. Read/Write/Edit/Glob/Grep — direct file operations and fallback when Codebase Memory/LSP is unavailable or too narrow.
5. Context-mode (`ctx_batch_execute`, `ctx_search`, `ctx_execute`, `ctx_execute_file`, `ctx_fetch_and_index`, `ctx_index`, `ctx_purge`, `ctx_vault_graph`, `ctx_vault_index`, `ctx_graph_analyze`,   `ctx_complexity`, `ctx_dead_code`, `ctx_dep_graph`, `ctx_insight`, `ctx_stats`, `ctx_upgrade`, `ctx_connector_add`, `ctx_connector_list`, `ctx_connector_sync`, `ctx_context_pack`,   `ctx_index_embeddings`, `ctx_semantic_search`) — large output, analysis, indexing
6. AST grep (`mcp__plugin_oh-my-claudecode_t__ast_grep_search`, `mcp__plugin_oh-my-claudecode_t__ast_grep_replace`) — structural code search and replace

### External & Network

6. GitHub plugin (`mcp__github__*`) — repo ops, issues, PRs
7. DDG Search MCP (`mcp__ddg-search__search`) — web search (no API key needed)
8. Tavily (`mcp__tavily__tavily_search`, `mcp__tavily__tavily_extract`, `mcp__tavily__tavily_research`, `mcp__tavily__tavily_crawl`, `mcp__tavily__tavily_map`) — web search (fallback)
9. Fetch MCP (`mcp__fetch__fetch_markdown`) — URL content (fallback: tavily-extract)
10. Playwright (`mcp__plugin_playwright_playwright__*`) — browser/UI automation (No fallback)

## MCP Category Assignments

### External Services

| Category | Primary | Fallback |
|----------|---------|----------|
| Document Lookup | context7 (`mcp__plugin_context7_context7__query-docs`) | DDG Search / Tavily |
| Web Content | fetch (`mcp__fetch__fetch_markdown`, `mcp__fetch__fetch_txt`) | Tavily (`mcp__tavily__tavily_extract`) |
| Web Search | DDG Search (`mcp__ddg-search__search`) | Tavily (`mcp__tavily__tavily_search`, `mcp__tavily__tavily_extract`, `mcp__tavily__tavily_research`, `mcp__tavily__tavily_crawl`, `mcp__tavily__tavily_map`) |
| Repo Operations | GitHub (`mcp__github__*`) | gh CLI via Bash |
| Browser Automation | Playwright (`mcp__plugin_playwright_playwright__*`) | No fallback |

### Code & Analysis

| Category | Primary | Fallback |
|----------|---------|----------|
| Large Output | context-mode (`ctx_batch_execute`, `ctx_search`) | No fallback |
| Code Intelligence | LSP (`lsp_*`) | Grep/Glob |
| Codebase Discovery | Codebase Memory (`mcp__codebase-memory__search_graph`, `mcp__codebase-memory__trace_path`, `mcp__codebase-memory__get_code_snippet`) | Grep/Glob |
| Structural Code | AST grep (`mcp__plugin_oh-my-claudecode_t__ast_grep_*`) | Grep |

### State & Runtime

| Category | Primary | Fallback |
|----------|---------|----------|
| State Management | OMC (`state_read`, `state_write`, `state_*`) | No fallback |
| Notepad | OMC (`notepad_read`, `notepad_write_*`) | No fallback |
| Python Runtime | OMC (`mcp__plugin_oh-my-claudecode_t__python_repl`) | Bash (python3) |

## Error Recovery (per MCP server)

- Context7 fail: retry resolve-library-id once → fallback to DDG Search MCP.
- DDG Search fail: retry once → fallback to Tavily MCP (`mcp__tavily__tavily_search`).
- Tavily fail: retry once → fallback to Fetch MCP for known URLs.
- Fetch MCP fail: retry once → no further fallback (URL unavailable).
- Playwright fail: retry once with `browser_navigate` → no fallback (manual browser required).
- GitHub plugin fail: fallback to `gh` CLI via Bash immediately.
- LSP disconnected: Grep/Glob fallback immediately.
- Codebase Memory fail: retry once → fallback to Grep/Glob + Read.
- Codebase Memory index_repository (Windows): use uppercase drive letter (`C:/`,`D:/`,`E:/` not `c:/`,`d:/`,`e:/`) or server rejects path as `store.corrupt` and `artifact.export` fails; `list_projects` may not see the project due to path case mismatch.
- Context-mode fail: retry once → fallback to Bash with output redirected to file.
- Agent error: retry with clearer prompt once, escalate after 2nd failure.
- WebSearch tool call fails: use DDG MCP instead.
- MCP servers can fail — assume they will. Every critical path must have a fallback. No fallback means no reliable path. [Murphy]
- Context overflow is a failure mode: context-mode is mandatory for large outputs. General Ordering does not override context-mode when output exceeds 20 lines.

## MCP Shorthand Convention

LSP, OMC State/Notepad, and AST tools listed as `lsp_*`, `state_*`, `notepad_*`, `ast_grep_*` are server-native tools (not prefixed with `mcp__`). All external MCP servers use full `mcp__*` prefix. When invoking, use the exact tool name from this table.

## MCP Config Reference

- MCP servers configured in `~/.claude.json` (NOT settings.json). The `enabled` array must list each server name.
- On Windows: use `C:/Program Files/nodejs/npx.cmd` as command.
- Restart Claude Code after adding/modifying MCP servers.
- Firecrawl API keys: `E:\tavily-key-generator` with `EMAIL_PROVIDER=duckmail`.

## MCP Tool Name Reference

| Server | Tools |
|--------|-------|
| context7 | `mcp__plugin_context7_context7__resolve-library-id`, `mcp__plugin_context7_context7__query-docs` |
| DDG Search | `mcp__ddg-search__search`, `mcp__ddg-search__fetch_content` |
| Tavily | `mcp__tavily__tavily_search`, `mcp__tavily__tavily_extract`, `mcp__tavily__tavily_research`, `mcp__tavily__tavily_crawl`, `mcp__tavily__tavily_map` |
| Fetch | `mcp__fetch__fetch_html`, `mcp__fetch__fetch_json`, `mcp__fetch__fetch_markdown`, `mcp__fetch__fetch_txt` |
| Playwright | `mcp__plugin_playwright_playwright__browser_*` (all browser automation tools: click, close, console_messages, drag, drop, evaluate, file_upload, fill_form, handle_dialog, hover, navigate, navigate_back, network_request, network_requests, press_key, resize, run_code_unsafe, select_option, snapshot, tabs, take_screenshot, type, wait_for) |
| GitHub | `mcp__github__add_issue_comment`, `mcp__github__create_branch`, `mcp__github__create_issue`, `mcp__github__create_or_update_file`, `mcp__github__create_pull_request`, `mcp__github__create_pull_request_review`, `mcp__github__create_repository`, `mcp__github__fork_repository`, `mcp__github__get_file_contents`, `mcp__github__get_issue`, `mcp__github__get_pull_request`, `mcp__github__get_pull_request_comments`, `mcp__github__get_pull_request_files`, `mcp__github__get_pull_request_reviews`, `mcp__github__get_pull_request_status`, `mcp__github__list_commits`, `mcp__github__list_issues`, `mcp__github__list_pull_requests`, `mcp__github__merge_pull_request`, `mcp__github__push_files`, `mcp__github__search_code`, `mcp__github__search_issues`, `mcp__github__search_repositories`, `mcp__github__search_users`, `mcp__github__update_issue`, `mcp__github__update_pull_request_branch` |
| Codebase Memory | `mcp__codebase-memory__index_repository`, `mcp__codebase-memory__search_graph`, `mcp__codebase-memory__query_graph`, `mcp__codebase-memory__trace_path`, `mcp__codebase-memory__get_code_snippet`, `mcp__codebase-memory__get_graph_schema`, `mcp__codebase-memory__get_architecture`, `mcp__codebase-memory__search_code`, `mcp__codebase-memory__list_projects`, `mcp__codebase-memory__delete_project`, `mcp__codebase-memory__index_status`, `mcp__codebase-memory__detect_changes`, `mcp__codebase-memory__manage_adr`, `mcp__codebase-memory__ingest_traces` |
| Context-Mode | `mcp__plugin_context-mode_context-mode__ctx_batch_execute`, `mcp__plugin_context-mode_context-mode__ctx_complexity`, `mcp__plugin_context-mode_context-mode__ctx_connector_add`, `mcp__plugin_context-mode_context-mode__ctx_connector_list`, `mcp__plugin_context-mode_context-mode__ctx_connector_sync`, `mcp__plugin_context-mode_context-mode__ctx_context_pack`, `mcp__plugin_context-mode_context-mode__ctx_dead_code`, `mcp__plugin_context-mode_context-mode__ctx_dep_graph`, `mcp__plugin_context-mode_context-mode__ctx_doctor`, `mcp__plugin_context-mode_context-mode__ctx_execute`, `mcp__plugin_context-mode_context-mode__ctx_execute_file`, `mcp__plugin_context-mode_context-mode__ctx_fetch_and_index`, `mcp__plugin_context-mode_context-mode__ctx_graph_analyze`, `mcp__plugin_context-mode_context-mode__ctx_index`, `mcp__plugin_context-mode_context-mode__ctx_index_embeddings`, `mcp__plugin_context-mode_context-mode__ctx_insight`, `mcp__plugin_context-mode_context-mode__ctx_purge`, `mcp__plugin_context-mode_context-mode__ctx_search`, `mcp__plugin_context-mode_context-mode__ctx_semantic_search`, `mcp__plugin_context-mode_context-mode__ctx_stats`, `mcp__plugin_context-mode_context-mode__ctx_upgrade`, `mcp__plugin_context-mode_context-mode__ctx_vault_graph`, `mcp__plugin_context-mode_context-mode__ctx_vault_index` |

## Codebase Memory Usage Policy

This policy applies to the orchestrator and to every subagent that performs codebase discovery.
Subagents must not assume the orchestrator has already completed discovery unless the prompt
explicitly provides the relevant Codebase Memory results, qualified names, or file paths.

If the orchestrator delegates discovery to a subagent, the delegation prompt must either:
1. include explicit Codebase Memory preflight instructions, or
2. provide already verified Codebase Memory results and tell the subagent to continue from them.

The first observable discovery tool call must be Codebase Memory, not Bash, Glob,
Grep, Read, or LSP. Planning text is not enough: if the reasoning says Codebase
Memory will be used, the next discovery action must actually call Codebase Memory
or explicitly report that the tool is unavailable.

Use Codebase Memory before Grep/Read/LSP when the task asks to:
- find where behavior is implemented;
- understand module architecture;
- trace call chains or data flow;
- modify code across multiple files;
- refactor classes, modules, events, DTOs, interfaces, or storage layers;
- answer "what depends on X?", "where is X used?", "how does X work?";
- inspect unfamiliar repository areas.

Preferred sequence:
1. `mcp__codebase-memory__list_projects` or `mcp__codebase-memory__index_status`
   if project/index state is unclear.
2. `mcp__codebase-memory__search_graph` for entities/modules.
3. `mcp__codebase-memory__trace_path` for call/data flow.
4. `mcp__codebase-memory__get_code_snippet` for relevant code.
5. Then use LSP/Read/Grep for exact edits and verification.

Do not skip Codebase Memory merely because Grep might find a keyword.
Grep is fallback or precision confirmation, not first-pass architecture discovery.
