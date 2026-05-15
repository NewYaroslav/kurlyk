# Tool Priority Chain

## Web Search (non-Anthropic provider override)
Native WebSearch does NOT work with non-Anthropic providers (Fireworks, OpenRouter, etc.).
Use MCP search tools instead. Priority:
1. DDG Search MCP (`mcp__ddg-search__search`, `mcp__ddg-search__fetch_content`) — no API key needed
2. Tavily MCP (`mcp__tavily__tavily_search`, `mcp__tavily__tavily_extract`, `mcp__tavily__tavily_research`, `mcp__tavily__tavily_crawl`, `mcp__tavily__tavily_map`) — requires API key, has free tier
3. Fetch MCP (`mcp__fetch__fetch_markdown`, `mcp__fetch__fetch_txt`, `mcp__fetch__fetch_html`, `mcp__fetch__fetch_json`) — for known URLs only
NEVER use built-in WebSearch tool — it will fail with non-Anthropic providers.

## Mandatory Overrides (always applies first)
- Context-mode: output >20 lines — mandatory, overrides General Ordering
- WebSearch: NEVER use built-in tool — always MCP (DDG > Tavily > Fetch)

## General Ordering (use first match)

### Local & Code Intelligence
1. Context7 (`mcp__plugin_context7_context7__resolve-library-id`, `mcp__plugin_context7_context7__query-docs`) — SDK/API docs before web search
2. Read/Write/Edit/Glob/Grep — file operations
3. Context-mode (`ctx_batch_execute`, `ctx_search`, `ctx_execute`, `ctx_execute_file`, `ctx_fetch_and_index`, `ctx_index`, `ctx_purge`, `ctx_vault_graph`, `ctx_vault_index`, `ctx_graph_analyze`, `ctx_complexity`, `ctx_dead_code`, `ctx_dep_graph`, `ctx_insight`, `ctx_stats`, `ctx_upgrade`, `ctx_connector_add`, `ctx_connector_list`, `ctx_connector_sync`, `ctx_context_pack`, `ctx_index_embeddings`, `ctx_semantic_search`) — large output, analysis, indexing
4. LSP (`lsp_hover`, `lsp_goto_definition`, `lsp_find_references`, `lsp_diagnostics`, `lsp_code_actions`, `lsp_code_action_resolve`, `lsp_document_symbols`, `lsp_workspace_symbols`, `lsp_prepare_rename`, `lsp_rename`, `lsp_servers`) — symbols, definitions, diagnostics
5. AST grep (`mcp__plugin_oh-my-claudecode_t__ast_grep_search`, `mcp__plugin_oh-my-claudecode_t__ast_grep_replace`) — structural code search and replace

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
| Structural Code | AST grep (`mcp__plugin_oh-my-claudecode_t__ast_grep_*`) | Grep |

### State & Runtime

| Category | Primary | Fallback |
|----------|---------|----------|
| State Management | OMC (`state_read`, `state_write`, `state_*`) | No fallback |
| Notepad | OMC (`notepad_read`, `notepad_write_*`) | No fallback |
| Python Runtime | OMC (`mcp__plugin_oh-my-claudecode_t__python_repl`) | Bash (python3) |

## Error Recovery (per MCP server)
- Context7 fail: retry resolve-library-id once → fallback to DDG Search MCP
- DDG Search fail: retry once → fallback to Tavily MCP (`mcp__tavily__tavily_search`)
- Tavily fail: retry once → fallback to Fetch MCP for known URLs
- Fetch MCP fail: retry once → no further fallback (URL unavailable)
- Playwright fail: retry once with `browser_navigate` → no fallback (manual browser required)
- GitHub plugin fail: fallback to `gh` CLI via Bash immediately
- LSP disconnected: Grep/Glob fallback immediately
- Context-mode fail: retry once → fallback to Bash with output redirected to file
- Agent error: retry with clearer prompt once, escalate after 2nd failure
- WebSearch tool call fails: use DDG MCP instead
- MCP-сервер МОЖЕТ упасть — упадёт. Каждый критический путь имеет fallback. Нет fallback = нет пути. [Murphy]
- Контекстный overflow = failure mode: context-mode mandatory. General Ordering НЕ отменяет context-mode при >20 строк.

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
| Context-Mode | `mcp__plugin_context-mode_context-mode__ctx_batch_execute`, `mcp__plugin_context-mode_context-mode__ctx_complexity`, `mcp__plugin_context-mode_context-mode__ctx_connector_add`, `mcp__plugin_context-mode_context-mode__ctx_connector_list`, `mcp__plugin_context-mode_context-mode__ctx_connector_sync`, `mcp__plugin_context-mode_context-mode__ctx_context_pack`, `mcp__plugin_context-mode_context-mode__ctx_dead_code`, `mcp__plugin_context-mode_context-mode__ctx_dep_graph`, `mcp__plugin_context-mode_context-mode__ctx_doctor`, `mcp__plugin_context-mode_context-mode__ctx_execute`, `mcp__plugin_context-mode_context-mode__ctx_execute_file`, `mcp__plugin_context-mode_context-mode__ctx_fetch_and_index`, `mcp__plugin_context-mode_context-mode__ctx_graph_analyze`, `mcp__plugin_context-mode_context-mode__ctx_index`, `mcp__plugin_context-mode_context-mode__ctx_index_embeddings`, `mcp__plugin_context-mode_context-mode__ctx_insight`, `mcp__plugin_context-mode_context-mode__ctx_purge`, `mcp__plugin_context-mode_context-mode__ctx_search`, `mcp__plugin_context-mode_context-mode__ctx_semantic_search`, `mcp__plugin_context-mode_context-mode__ctx_stats`, `mcp__plugin_context-mode_context-mode__ctx_upgrade`, `mcp__plugin_context-mode_context-mode__ctx_vault_graph`, `mcp__plugin_context-mode_context-mode__ctx_vault_index` |
