# Build And Test

## Build System

Use CMake. Preserve Windows MinGW compatibility when changing build scripts,
compiler options, or examples.

## Windows Workflow

On Windows, prefer the system MinGW toolchain and the `MinGW Makefiles`
generator unless a task explicitly asks for a different generator.

Typical flow:

```bash
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
ctest --test-dir build --output-on-failure
```

If MinGW is unavailable, use another local CMake generator and state the
generator in the final task result.

## Fallback Dependencies

When system packages are missing, enable bundled fallbacks:

```bash
cmake -S . -B build -G "MinGW Makefiles" \
  -DKURLYK_USE_FALLBACK_OPENSSL=ON \
  -DKURLYK_USE_FALLBACK_CURL=ON \
  -DKURLYK_USE_FALLBACK_ASIO=ON \
  -DKURLYK_USE_FALLBACK_SIMPLE_WS_SERVER=ON
cmake --build build
```

## Verification Expectations

- For C++ changes, run at least configure and build.
- Run `ctest --test-dir build --output-on-failure` when tests exist.
- Build examples when the touched behavior affects documented usage.
- If a check cannot be run because a local tool is missing, say that explicitly
  instead of treating it as a pass.

## Quick Example Compile

To verify header-only builds without full CMake:

```bash
g++ examples/simple_http_request_example.cpp -Iinclude -std=c++17 \
    -pthread -lcurl -lssl -lcrypto -DKURLYK_WEBSOCKET_SUPPORT=0 -o simple_http_example
./simple_http_example
```

## Doxygen

```bash
doxygen Doxyfile
```
