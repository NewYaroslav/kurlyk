# AGENTS.md

## Overview

**kurlyk** is a header-only C++11/17 library built on top of **libcurl** and **Simple-WebSocket-Server** to provide convenient HTTP and WebSocket clients. It hides networking boilerplate and offers asynchronous execution, rate limiting, automatic reconnection, and proxy support. The `core::NetworkWorker` processes tasks in a background thread while modules such as `HttpClient` and `WebSocketClient` expose simple class-based APIs.

## Features

- Asynchronous HTTP requests and WebSocket messaging
- Optional background worker or synchronous processing
- Rate limiting and retry logic for both transports
- Automatic WebSocket reconnection with configurable attempts
- Proxy servers, custom headers and timeouts
- Centralized error dispatching through `NetworkWorker`
- Header-only design compatible with C++11 and later

## Use Cases

- REST/HTTP clients for small applications or bots
- WebSocket clients that send/receive messages with reconnection logic
- Lightweight utilities needing rate-limited network access
- Experiments and tools where manual networking boilerplate is undesirable

## Installation & Build

- Add `include/` to your compiler's include path and link against `libcurl`, `OpenSSL`, and either Boost.Asio or standalone Asio. WebSocket functionality additionally requires Simple-WebSocket-Server.
- Examples live in `examples/` and rely on libraries shipped in `libs/`; you may use system packages instead.
- To build `simple_http_request_example` with only HTTP enabled:

```bash
g++ examples/simple_http_request_example.cpp -Iinclude -std=c++17 \
    -pthread -lcurl -lssl -lcrypto -DKURLYK_WEBSOCKET_SUPPORT=0 -o simple_http_example
./simple_http_example
```

- Generate Doxygen documentation:

```bash
doxygen Doxyfile
```

## Configuration Macros

Define these macros before including `<kurlyk.hpp>` to tailor functionality. "0" means disabled, "1" enabled unless noted.

| Macro | Values | Default | Description |
|-------|--------|---------|-------------|
| `KURLYK_AUTO_INIT` | 0 / 1 | 1 | Automatically register managers during static initialization. |
| `KURLYK_AUTO_INIT_USE_ASYNC` | 0 / 1 | 1 | Run `NetworkWorker` in a background thread during auto-init. Ignored if `KURLYK_AUTO_INIT` = 0. |
| `KURLYK_HTTP_SUPPORT` | 0 / 1 | 1 | Include HTTP components such as `HttpClient`. |
| `KURLYK_WEBSOCKET_SUPPORT` | 0 / 1 | 1 | Include WebSocket components such as `WebSocketClient`. |
| `KURLYK_ENABLE_JSON` | 0 / 1 | 0 | Include nlohmann::json and expose JSON-aware types. |
| `KURLYK_USE_JSON` | defined / undefined | undefined | Enable enum ↔ JSON helpers in `type_utils.hpp`; usually set alongside `KURLYK_ENABLE_JSON`. |
| `KURLYK_USE_CURL` | defined / undefined | defined on non-Emscripten | Use libcurl for HTTP features. |
| `KURLYK_USE_SIMPLEWEB` | defined / undefined | defined on non-Emscripten | Use Simple-WebSocket-Server for WebSocket features. |
| `KURLYK_USE_EMSCRIPTEN` | defined / undefined | defined when compiling for Emscripten | Use Emscripten-specific WebSocket adapters instead of curl/SimpleWeb. |

## Core Components

| Component | Description |
|-----------|-------------|
| `core::NetworkWorker` | Singleton that processes HTTP and WebSocket tasks in a background thread. |
| `HttpClient` | Sends asynchronous HTTP requests with rate limits, proxy and retry support. |
| `WebSocketClient` | Manages WebSocket connections, event handlers and message sending. |
| `HttpRequestManager` / `WebSocketManager` | Internal managers coordinating requests and applying rate limits. |

## Testing and Build

There is no dedicated test suite. When modifying library headers, compile at least one example from `examples/` (e.g., `simple_http_request_example.cpp`) to ensure the code still builds.

## Code Style: Git Commit Convention

Follow the [Conventional Commits](https://www.conventionalcommits.org/) style:

- `feat:` new features
- `fix:` bug fixes
- `docs:` documentation changes
- `refactor:` code refactoring without behaviour changes
- `test:` when adding or modifying tests

Format: `type(scope): short description` where the scope is optional. Keep messages short and imperative.

