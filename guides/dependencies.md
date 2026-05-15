# Dependencies

`kurlyk` is header-only but requires linked libraries for transport and crypto.

| Dependency | Role | Fallback |
|---|---|---|
| libcurl | HTTP transport | `DKURLYK_USE_FALLBACK_CURL=ON` |
| OpenSSL | TLS for curl | `DKURLYK_USE_FALLBACK_OPENSSL=ON` |
| Asio (Boost or standalone) | WebSocket I/O | `DKURLYK_USE_FALLBACK_ASIO=ON` |
| Simple-WebSocket-Server | WebSocket protocol layer | `DKURLYK_USE_FALLBACK_SIMPLE_WS_SERVER=ON` |
| nlohmann/json | Optional JSON support behind `KURLYK_ENABLE_JSON` | None (disabled by default) |
| Emscripten | Alternative transport for web builds | Enabled automatically when compiling for Emscripten |

## Policy

- Prefer system packages; use bundled copies in `external/` only when the system
  package is unavailable.
- Do not edit vendored code under `external/` unless the task is an explicit
  dependency update.
- Optional features stay behind compile definitions:
  `KURLYK_ENABLE_JSON`, `KURLYK_WEBSOCKET_SUPPORT`.
