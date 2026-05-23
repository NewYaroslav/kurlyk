# Project Overview

## Domain

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

## Core Components

| Component | Description |
|-----------|-------------|
| `core::NetworkWorker` | Singleton that processes HTTP and WebSocket tasks in a background thread. |
| `HttpClient` | Sends asynchronous HTTP requests with rate limits, proxy and retry support. |
| `WebSocketClient` | Manages WebSocket connections, event handlers and message sending. |
| `HttpRequestManager` / `WebSocketManager` | Internal managers coordinating requests and applying rate limits. |

## Configuration Macros

Define these macros before including `<kurlyk.hpp>` to tailor functionality. "0" means disabled, "1" enabled unless noted.

| Macro | Values | Default | Description |
|-------|--------|---------|-------------|
| `KURLYK_AUTO_INIT` | 0 / 1 | 1 | Automatically register managers during static initialization. |
| `KURLYK_AUTO_INIT_USE_ASYNC` | 0 / 1 | 1 | Run `NetworkWorker` in a background thread during auto-init. Ignored if `KURLYK_AUTO_INIT` = 0. |
| `KURLYK_HTTP_SUPPORT` | 0 / 1 | 1 | Include HTTP components such as `HttpClient`. |
| `KURLYK_WEBSOCKET_SUPPORT` | 0 / 1 | 1 | Include WebSocket components such as `WebSocketClient`. |
| `KURLYK_JSON_SUPPORT` | 0 / 1 | 0 | Include nlohmann::json and expose JSON-aware types and enum helpers. |
| `KURLYK_USE_CURL` | defined / undefined | defined on non-Emscripten | Use libcurl for HTTP features. |
| `KURLYK_USE_SIMPLEWEB` | defined / undefined | defined on non-Emscripten | Use Simple-WebSocket-Server for WebSocket features. |
| `KURLYK_USE_EMSCRIPTEN` | defined / undefined | defined when compiling for Emscripten | Use Emscripten-specific WebSocket adapters instead of curl/SimpleWeb. |
