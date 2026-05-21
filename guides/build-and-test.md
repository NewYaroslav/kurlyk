# Build and Test

## Dependencies

- **libcurl** — HTTP transport
- **OpenSSL** — TLS for both HTTP and WebSocket
- **Boost.Asio** or **standalone Asio** — WebSocket I/O
- **Simple-WebSocket-Server** — WebSocket protocol layer

All dependencies are bundled as git submodules under `libs/` and can also be provided by the system.

## Quick Build

Add `include/` to your compiler's include path and link against the libraries above. Examples live in `examples/`.

Build all repository targets with CMake:

```bash
cmake -S . -B build-examples -DKURLYK_BUILD_EXAMPLES=ON
cmake --build build-examples --config Release
```

For MinGW, select the generator and compilers explicitly:

```bash
cmake -S . -B build-examples-mingw -G "MinGW Makefiles" \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DKURLYK_BUILD_EXAMPLES=ON
cmake --build build-examples-mingw --config Release
```

### Minimal manual compilation (HTTP only)

```bash
g++ examples/simple_http_request_example.cpp -Iinclude -std=c++17 \
    -pthread -lcurl -lssl -lcrypto -DKURLYK_WEBSOCKET_SUPPORT=0 -o simple_http_example
./simple_http_example
```

## Fallback Dependencies

CMake can automatically download missing dependencies. Availability depends on the compiler and linkage type.

| Dependency | MinGW (Shared) | MinGW (Static) | MSVC (Shared) | MSVC (Static) |
|------------|---------------|---------------|---------------|---------------|
| OpenSSL    | yes           | yes           | yes           | yes           |
| curl       | yes           | yes           | yes           | no            |

Asio and Simple-WebSocket-Server are header-only and work for all build variants.

### CMake fallback options

| Option | Description |
|--------|-------------|
| `KURLYK_USE_FALLBACK_OPENSSL` | Enables OpenSSL fallback. |
| `KURLYK_USE_FALLBACK_CURL` | Enables libcurl fallback. |
| `KURLYK_USE_FALLBACK_ASIO` | Enables Asio fallback. |
| `KURLYK_USE_FALLBACK_SIMPLE_WS_SERVER` | Enables Simple-WebSocket-Server fallback. |
| `KURLYK_OPENSSL_SHARED` | Loads OpenSSL as a shared library when fallback is enabled. |
| `KURLYK_CURL_SHARED` | Loads libcurl as a shared library when fallback is enabled. |
| `KURLYK_BUILD_EXAMPLES` | Builds all targets from the `examples/` directory. |

## Testing

There is no dedicated unit test suite. When modifying library headers, compile at least one example from `examples/` (e.g., `simple_http_request_example.cpp`) to ensure the code still builds.

### Windows integration suite

```powershell
powershell -ExecutionPolicy Bypass -File tests/integration/run_integration_tests.ps1
```

### ODR suite

```powershell
powershell -ExecutionPolicy Bypass -File tests/odr/run_odr_tests.ps1
```

### Portable smoke test

```bash
c++ tests/smoke/header_smoke.cpp -Iinclude -std=c++11 -o header_smoke
./header_smoke

c++ tests/smoke/header_smoke.cpp -Iinclude -std=c++17 -o header_smoke
./header_smoke
```

## CI Coverage

| Platform | Coverage |
|----------|----------|
| Windows | MinGW and MSVC integration builds with fallback dependencies, HTTP backpressure regression, and local WebSocket integration coverage. |
| Windows extras | ODR checks for singleton and auto-initialization headers. |
| Linux | C++11/C++17 header smoke test with HTTP/WebSocket disabled. |
| macOS | C++11/C++17 header smoke test with HTTP/WebSocket disabled. |

## Documentation

Generate Doxygen documentation:

```bash
doxygen Doxyfile
```

Published documentation: <https://newyaroslav.github.io/kurlyk/>.
