# Codebase Orientation

Use this file when you need a fast, project-specific mental model before
changing `kurlyk`. It complements the narrower playbooks in `guides/` and does not
replace them.

## Quick Model

`kurlyk` is a header-only C++11/17 library that wraps **libcurl** and
**Simple-WebSocket-Server** into asynchronous HTTP and WebSocket clients. Its
public surface is a class-based API (`HttpClient`, `WebSocketClient`) backed by
singleton managers (`HttpRequestManager`, `WebSocketManager`) that queue,
rate-limit, retry, and dispatch work through `core::NetworkWorker`.

The project is not a DDD application. There are no database layers, event buses,
repositories, or product domains. Treat the "domains" below as library
subsystems.

## Public Entry Points

Prefer these umbrella headers instead of recreating include order manually:

| Header | Purpose |
| --- | --- |
| `include/kurlyk.hpp` | Full library entry point: config, core, types, HTTP, WebSocket, utilities, and startup. |
| `include/kurlyk/http/HttpClient.hpp` | Public HTTP client with fluent request builder. |
| `include/kurlyk/websocket/WebSocketClient.hpp` | Public WebSocket client with connect/send/receive/callbacks. |

Leaf headers may be included directly when a consumer needs only part of the
API. Internal subsystems include from `include/kurlyk/http/data.hpp` for request
and response DTOs.

## Subsystems

| Subsystem | Main files | Responsibility |
| --- | --- | --- |
| Macro facade / auto-init | `startup/auto_init.hpp` | Optionally registers managers and starts `NetworkWorker` at static-init time. Gated by `KURLYK_AUTO_INIT`. |
| Core worker | `core/NetworkWorker.hpp`, `core/INetworkTaskManager.hpp` | Singleton background thread that processes tasks from registered managers. |
| HTTP client | `http/HttpClient.hpp` | Fluent builder API; owns proxy, timeout, rate-limit, retry, and callback settings. |
| HTTP request data | `http/data/HttpRequest.hpp`, `http/data/HttpResponse.hpp`, `http/data.hpp` | DTOs for requests, responses, headers, query params, and multipart form data. |
| HTTP request manager | `http/HttpRequestManager.hpp` | Singleton queue, rate-limiter, retry logic, batch executor, and cancellation for HTTP. |
| HTTP batch handler | `http/HttpRequestManager/HttpBatchRequestHandler.hpp` | Executes one or more active requests via libcurl multi interface. |
| HTTP rate limiter | `http/HttpRequestManager/HttpRateLimiter.hpp`, `HttpRateLimitHandle.hpp` | Partitioned rate-limit engine: per-ID count/period limits + optional sequential mode, now keyed by string partition. |
| HTTP utilities | `http/utils.hpp` | Factory helpers for creating requests and setting defaults. |
| WebSocket client | `websocket/WebSocketClient.hpp` | Connect, send, close, and event callbacks with auto-reconnect. |
| WebSocket manager | `websocket/WebSocketManager.hpp` | Internal queue and connection state for WebSocket. |
| Shared types | `types/*.hpp`, `types.hpp` | Enums (`ContentType`, `WebSocketOpcode`, `ClientError`), proxy configs, status helpers. |
| Core utilities | `utils/error_utils.hpp`, `utils/type_utils.hpp`, `utils.hpp` | Error categories, enum helpers, and optional JSON conversions. |

## Layer Rules

The practical dependency direction is:

1. `config.hpp` provides compile-time feature flags.
2. `types.hpp` provides shared public constants, enums, and DTOs.
3. `core/` depends only on `types/` and standard headers.
4. `http/` and `websocket/` depend on `core/` and `types/`, then expose public
   clients and internal managers.
5. `startup/` wires auto-init over `core/` + `http/` + `websocket/`.

Avoid these dependency shapes:

- Public leaf headers directly including sibling modules when the nearest
  umbrella exists.
- `utils/` including `http/` or `websocket/` internals.
- New code that requires users to include files in a fragile custom order.

Namespaces:

- Public API lives in `namespace kurlyk`.
- Private implementation details stay in anonymous namespaces or `detail`
  sub-namespaces where appropriate.

## Design Patterns In Use

- **Facade**: `HttpClient` hides `HttpRequest`, `HttpRequestManager`, rate-limit
  handles, and `NetworkWorker` behind a fluent builder.
- **Singleton**: `NetworkWorker::get_instance()`, `HttpRequestManager::get_instance()`,
  and `WebSocketManager::get_instance()` are process-wide coordinators. They
  intentionally use static pointer singletons to survive shutdown/static-destruction
  ordering.
- **RAII handles**: `HttpRateLimitHandle` keeps a rate-limit entry alive while
  any request or manager still references it. Physical erase is deferred until
  the last handle is destroyed.
- **Strategy / builder**: `HttpClient` accumulates request parameters, then
  materializes an `HttpRequest` on `perform_request()`.
- **DTOs**: `HttpRequest`, `HttpResponse`, `QueryParams`, `HttpHeader`, `PartData`
  are small value-type structs.

## Utility Inventory

Reuse these helpers instead of writing local equivalents:

| Need | Reuse |
| --- | --- |
| Create a default request | `utils::create_http_request()` in `http/utils.hpp`. |
| Check status-code category | `utils::is_success(...)`, `is_redirect(...)`, `is_client_error(...)` in `utils/error_utils.hpp`. |
| Convert enums to/from string | `utils::to_string(...)`, `from_string(...)` in `utils/type_utils.hpp`. |
| JSON conversions (optional) | `utils::to_json(...)`, `from_json(...)` behind `KURLYK_ENABLE_JSON`. |

## Extension Recipes

### Add a Public Data Struct

Use this for DTO-style data exposed by HTTP/WebSocket APIs.

1. Add a PascalCase header in `include/kurlyk/http/data/`, for example
   `HttpCacheEntry.hpp`.
2. Put the struct in `namespace kurlyk`, give fields safe defaults, and
   document it with `/// \\brief`.
3. Include it from `include/kurlyk/http/data.hpp`.
4. Update `README.md`, `README-RU.md`, and `docs/mainpage.dox` if the DTO
   affects public behavior.

### Add a Configuration Macro

Use this when adding a compile-time feature toggle.

1. Add the macro to `include/kurlyk/config.hpp` with a safe default.
2. Gate any new code paths with `#ifdef` or `#if` checks.
3. Update the macro table in `AGENTS.md` (now in `guides/codebase-orientation.md`
   under Configuration Macros).
4. Update `README.md` and `README-RU.md`.

## Safety Invariants

- `core::NetworkWorker` processes tasks on a single background thread; do not
  block it with heavy computation or synchronous I/O.
- HTTP and WebSocket managers apply rate limits and retries consistently.
- Error dispatch flows through `NetworkWorker`; maintain exception safety in
  callbacks.
- Public APIs rely on value semantics and RAII.
- Rate-limit in-flight tokens must survive retry attempts; do not lose the
  `on_complete` callback that releases them when `allow_request()` temporarily
  denies a retried request.
- Code remains portable across C++11/17 compilers and network stacks.
- Optional dependency features must stay behind compile definitions.
- Do not edit vendored code under `libs/` except for an explicit dependency
  update task.

See `guides/concurrency.md` for full thread-safety contracts, callback/mutex
ordering rules, and shutdown/cancellation invariants.

## Project Map

| Path | Purpose | Edit when |
| --- | --- | --- |
| `AGENTS.md` | Root agent rules and playbook index. | Agent workflow or repository-wide guidance changes. |
| `guides/` | Detailed agent playbooks. | A recurring agent workflow needs documented project-specific rules. |
| `include/kurlyk.hpp` | Full umbrella header. | Public entry-point composition changes. |
| `include/kurlyk/config.hpp` | Compile-time feature flags. | Adding compile-time knobs. |
| `include/kurlyk/core/` | `NetworkWorker`, `INetworkTaskManager`. | Core task scheduling or threading changes. |
| `include/kurlyk/types/` | Shared enums, proxy config, status helpers. | Adding public constants or config types. |
| `include/kurlyk/http/` | HTTP client, request manager, rate limiter, batch handler, data DTOs. | Any HTTP feature, API, or internal mechanic changes. |
| `include/kurlyk/websocket/` | WebSocket client and manager. | Any WebSocket feature or connection logic changes. |
| `include/kurlyk/utils/` | Error categories, enum helpers, optional JSON. | Adding shared utilities. |
| `include/kurlyk/startup/` | Auto-init macros and registration. | Changing initialization behavior. |
| `tests/` | Integration tests. | Adding or changing behavior tests. |
| `examples/` | Usage examples. | Public workflow or new feature examples. |
| `docs/` | Doxygen mainpage and architecture notes. | Public documentation or deeper design notes. |
| `.github/workflows/` | CI and publishing workflows. | Build matrix, verification, or release automation changes. |
| `libs/` | Vendored fallback dependencies. | Explicit dependency updates only. |

## Usually Avoid Editing Without Need

- `libs/` vendor trees.
- Generated or local build trees such as `build/`, `build-mingw*/`, and `tmp/`.
- `docs/html/` or `docs/latex/` generated Doxygen output.
- Public macro names, enum values, and include paths unless the task is a
  deliberate breaking change.
- `HttpRateLimitLease.hpp` — deprecated unused file; do not revive without a
  specific architectural decision.

## Final Checklist For Agents

- Read the narrow playbook that matches the task:
  `build-and-test.md`, `cpp-development-guidelines.md`,
  `header-implementation-guidelines.md`, or `commit-conventions.md`.
- Prefer umbrella headers and preserve include policy.
- Keep C++11 compatibility unless the code path is explicitly C++17-gated.
- Keep async and callback APIs thread-safe.
- Add tests matching the changed surface.
- Update README, Doxygen, examples, or CI metadata when public behavior changes.
