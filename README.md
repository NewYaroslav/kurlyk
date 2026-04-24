# kurlyk
![logo](docs/logo-mini.png)

**C++ library for easy networking**

[![MIT License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)
![C++ Standard](https://img.shields.io/badge/C++-11--17-orange)
![CI Windows](https://img.shields.io/github/actions/workflow/status/NewYaroslav/kurlyk/ci.yml?branch=main&label=Windows&logo=windows)
![CI Linux](https://img.shields.io/github/actions/workflow/status/NewYaroslav/kurlyk/ci.yml?branch=main&label=Linux&logo=linux)
![CI macOS](https://img.shields.io/github/actions/workflow/status/NewYaroslav/kurlyk/ci.yml?branch=main&label=macOS&logo=apple)

[README на русском](README-RU.md)

## Description

**kurlyk** is another library implementing HTTP and WebSocket clients for C++. Built as a wrapper around `curl` and `Simple-WebSocket-Server`, it provides a simplified interface for working with HTTP and WebSocket connections in C++ applications. The library supports asynchronous HTTP requests with rate limiting and retry mechanisms, as well as WebSocket connectivity.

If you’re not satisfied with other libraries like *easyhttp-cpp, curl_request, curlpp-async, curlwrapper, curl-Easy-cpp, curlpp11, easycurl, curl-cpp-wrapper…* then you might want to give `kurlyk` a try.

### Features

- Asynchronous HTTP and WebSocket requests
- Optional background worker or synchronous processing
- Rate limiting support to prevent network overload
- Optional bounded queue admission to protect HTTP and WebSocket producers from unbounded growth
- Automatic reconnection with customizable parameters
- Proxy servers, custom headers, cookies and timeouts
- Simple and intuitive class-based interface
- Designed for use in small applications
- Supports C++11 and newer toolchains

### CI coverage

| Platform | Coverage |
|----------|----------|
| Windows | MinGW and MSVC integration builds with fallback dependencies, HTTP backpressure regression, and local WebSocket integration coverage. |
| Windows extras | ODR checks for singleton and auto-initialization headers. |
| Linux | C++11/C++17 header smoke test with HTTP/WebSocket disabled. |
| macOS | C++11/C++17 header smoke test with HTTP/WebSocket disabled. |

## Backpressure

Kurlyk now separates two forms of flow control:

- Rate limiting slows dispatch to the network backend.
- Backpressure limits how much work is accepted into selected queues.

Current bounded queues:

- HTTP uses a global pending-request queue limit configured with `kurlyk::set_max_pending_requests(...)`.
- WebSocket uses a per-client outbound send queue limit configured with `WebSocketClient::set_max_send_queue_size(...)`.
- A value of `0` keeps the queue unbounded.

Admission helpers:

- `kurlyk::SubmitResult`
- `kurlyk::submit_http_request(...)`
- `IWebSocketSender::submit_message(...)`
- `IWebSocketSender::submit_close(...)`
- `WebSocketClient::submit_message(...)`
- `WebSocketClient::submit_close(...)`

Compatibility notes:

- Existing `bool`-returning APIs are still available as wrappers.
- HTTP future overloads now become ready immediately on admission reject and return an `HttpResponse` with `error_code` instead of throwing a `runtime_error`.
- Backpressure in this pass does not bound `NetworkWorker`, WebSocket FSM queues, or stored event queues.

### HTTP backpressure example

```cpp
kurlyk::init(true);
kurlyk::set_max_pending_requests(64);

std::unique_ptr<kurlyk::HttpRequest> request(new kurlyk::HttpRequest());
request->request_id = kurlyk::generate_request_id();
request->method = "GET";
request->set_url("https://httpbin.org/get", kurlyk::QueryParams());

kurlyk::SubmitResult submit = kurlyk::submit_http_request(
    std::move(request),
    [](kurlyk::HttpResponsePtr response) {
        if (response && response->error_code) {
            std::cout << "HTTP runtime error: " << response->error_code.message() << std::endl;
        }
    });

if (!submit) {
    std::cout << "HTTP submit rejected: " << submit.error_code.message() << std::endl;
}
```

### WebSocket backpressure example

```cpp
kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");
client.set_max_send_queue_size(32);

kurlyk::SubmitResult submit = client.submit_message(
    "Hello with admission check",
    0,
    [](const std::error_code& ec) {
        if (ec) {
            std::cout << "Send failed: " << ec.message() << std::endl;
        }
    });

if (!submit) {
    std::cout << "WebSocket submit rejected: " << submit.error_code.message() << std::endl;
}
```

## Usage Examples

Examples are located in the `examples` folder. Below are some basic usage examples.

Build all repository examples with CMake:

```powershell
cmake -S . -B build-examples -DKURLYK_BUILD_EXAMPLES=ON
cmake --build build-examples --config Release
```

### WebSocket Client Example

This example shows how to connect to a WebSocket server, send a message, and handle various events (connection open, message received, connection close, and error). The default C++17 auto-initialization path is used here; for C++11/14 define `KURLYK_AUTO_INIT=0` and call `kurlyk::init()` / `kurlyk::deinit()` explicitly.

```cpp
#include <kurlyk.hpp>
#include <thread>
#include <chrono>

int main() {
    // Create a WebSocket client with the specified server URL
    kurlyk::WebSocketClient client("wss://echo-websocket.fly.dev/");

    // Set up WebSocket event handler
    client.on_event([](std::unique_ptr<kurlyk::WebSocketEventData> event) {
        switch (event->event_type) {
            case kurlyk::WebSocketEventType::WS_OPEN:
                KURLYK_PRINT << "Connection established" << std::endl;

                KURLYK_PRINT << "HTTP Version: " << event->sender->get_http_version() << std::endl;
                KURLYK_PRINT << "Headers:" << std::endl;
                for (const auto& header : event->sender->get_headers()) {
                    KURLYK_PRINT << header.first << ": " << header.second << std::endl;
                }

                event->sender->send_message("Hello, WebSocket!", 0, [](const std::error_code& ec) {
                    if (ec) {
                        KURLYK_PRINT << "Failed to send message: " << ec.message() << std::endl;
                    } else {
                        KURLYK_PRINT << "Message sent successfully" << std::endl;
                    }
                });
                break;

            case kurlyk::WebSocketEventType::WS_MESSAGE:
                KURLYK_PRINT << "Message received: " << event->message << std::endl;
                event->sender->send_message("Hello again!");
                break;

            case kurlyk::WebSocketEventType::WS_CLOSE:
                KURLYK_PRINT << "Connection closed: " << event->message
                             << "; Status code: " << event->status_code << std::endl;
                break;

            case kurlyk::WebSocketEventType::WS_ERROR:
                KURLYK_PRINT << "Error: " << event->error_code.message() << std::endl;
                break;
        }
    });

    KURLYK_PRINT << "Connecting..." << std::endl;
    client.connect();

    std::this_thread::sleep_for(std::chrono::seconds(10));

    KURLYK_PRINT << "Disconnecting..." << std::endl;
    client.disconnect_and_wait();

    KURLYK_PRINT << "End of execution" << std::endl;
    return 0;
}
```

### HTTP Client Examples

These examples demonstrate how to use the kurlyk HTTP client to perform different requests and handle responses. They disable auto-init because they call `kurlyk::init()` and `kurlyk::deinit()` manually.

#### Shared helper used by the examples

```cpp
#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>
#include <iostream>

void print_response(const kurlyk::HttpResponsePtr& response) {
    if (!response) {
        KURLYK_PRINT << "response is null" << std::endl;
        return;
    }

    KURLYK_PRINT
        << "ready: " << std::boolalpha << response->ready << std::endl
        << "response: " << response->content << std::endl
        << "error_code: " << response->error_code.message() << std::endl
        << "status_code: " << response->status_code << std::endl
        << "----------------------------------------" << std::endl;
}
```

#### Example 1: Performing GET and POST requests with response handlers

```cpp
int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    client.post("/post", kurlyk::QueryParams(), {{"Content-Type", "application/json"}},
        "{\"text\":\"Sample POST Content\"}",
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    kurlyk::deinit();
    return 0;
}
```

#### Example 2: Performing GET and POST requests with std::future

If a request is rejected before it enters the pending queue, the returned future becomes ready immediately and yields an `HttpResponse` with `error_code` set to `QueueLimitExceeded` or `ShuttingDown`.

```cpp
int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    auto future_response = client.get("/get", kurlyk::QueryParams{{"param", "value"}}, kurlyk::Headers());
    print_response(future_response.get());

    auto future_post = client.post("/post", kurlyk::QueryParams(),
        kurlyk::Headers{{"Header", "Value"}}, "Async POST Content");
    print_response(future_post.get());

    kurlyk::deinit();
    return 0;
}
```

#### Example 3: Setting up a proxy and sending a GET request

```cpp
int main() {
    kurlyk::init(true);
    kurlyk::HttpClient client("https://httpbin.org");

    client.set_proxy("127.0.0.1", 8080, "username", "password", kurlyk::ProxyType::HTTP);

    client.get("/ip", kurlyk::QueryParams(), kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();
    kurlyk::deinit();
    return 0;
}
```

#### Example 4: GET request using the `http_get` callback overload

```cpp
int main() {
    kurlyk::init(true);

    const uint64_t request_id = kurlyk::http_get(
        "https://httpbin.org/ip",
        kurlyk::QueryParams(),
        kurlyk::Headers(),
        [](const kurlyk::HttpResponsePtr response) {
            print_response(response);
        });

    KURLYK_PRINT << "Request id: " << request_id << std::endl;
    KURLYK_PRINT << "Press Enter to exit..." << std::endl;
    std::cin.get();

    kurlyk::cancel_request_by_id(request_id).wait();
    kurlyk::deinit();
    return 0;
}
```

#### Example 5: GET request using the `http_get` future overload

```cpp
int main() {
    kurlyk::init(true);

    auto result = kurlyk::http_get(
        "https://httpbin.org/ip",
        kurlyk::QueryParams(),
        kurlyk::Headers());

    KURLYK_PRINT << "Request id: " << result.first << std::endl;
    print_response(result.second.get());

    kurlyk::deinit();
    return 0;
}
```

## Dependencies and Installation

### Supported compiler toolchains

- **MSVC**
- **MinGW (GCC)**

MSVC support is currently considered unstable. The confirmed working configuration is **C++17** with **Visual Studio 2022 (generator: Visual Studio 17 2022)**.

### Dependencies

To work with the **kurlyk** library in MinGW, you will need the following dependencies:

1. For WebSocket:

    - [Simple-WebSocket-Server](https://gitlab.com/eidheim/Simple-WebSocket-Server)
    - Boost.Asio or [standalone Asio](https://github.com/chriskohlhoff/asio/tree/master)
    - [OpenSSL](https://slproweb.com/products/Win32OpenSSL.html) (*LTS version Win64 OpenSSL v3.0.15*)

2. For HTTP:
    - [libcurl](https://curl.se/windows/)

All dependencies are also included as submodules in the `libs` folder.

### OpenSSL Setup

1. Add OpenSSL paths to the project (example for version *3.4.0*):

```
OpenSSL-Win64/include
OpenSSL-Win64/lib/VC/x64/MD
OpenSSL-Win64/bin
```

2. Link OpenSSL libraries from `lib/VC/x64/MD`:

```
capi.lib
dasync.lib
libcrypto.lib
libssl.lib
openssl.lib
ossltest.lib
padlock.lib
```

### Standalone Asio Setup

1. Add the path to Asio in your project (example for [Asio repository](https://github.com/chriskohlhoff/asio/tree/master)):

```
asio/asio/include
```

2. Set the macro `ASIO_STANDALONE` in your project settings or before including `kurlyk.hpp`:

```cpp
#define ASIO_STANDALONE
#include <kurlyk.hpp>
```

> **Note:** For Boost.Asio, you do not need to define the `ASIO_STANDALONE` macro.

### curl Setup

1. Add the paths for `curl` in your project (example for version *8.11.0*):

```
curl-8.11.0_1-win64-mingw/bin
curl-8.11.0_1-win64-mingw/include
curl-8.11.0_1-win64-mingw/lib
```

2. Link the `curl` libraries from the `lib` folder:

```
libcurl.a
libcurl.dll.a
```

### Simple-WebSocket-Server Setup

Add the path to the Simple-WebSocket-Server headers in your project:

```
Simple-WebSocket-Server
```

### Linking Other Dependencies

Additionally, link the following libraries in the linker:

```
ws2_32
wsock32
crypt32
```

### Dependency resolution and fallbacks

The project provides a fallback mechanism for third-party dependencies. If a required dependency is not available on the system, it can be automatically downloaded and built as part of the project.
Fallback availability depends on the selected compiler toolchain and the linkage type (static or shared). The supported combinations are shown in the table below.

| Dependency | MinGW (Shared) | MinGW (Static) | MSVC (Shared) | MSVC (Static) |
|------------|---------------|---------------|---------------|---------------|
| OpenSSL    | yes           | yes           | yes           | yes           |
| curl       | yes           | yes           | yes           | no            |

Asio and Simple-WebSocket-Server are header-only libraries and suitable for all specified configuration types.

#### CMake fallback options

The following CMake options control the fallback mechanism:

| Option | Description |
|--------|-------------|
| `KURLYK_USE_FALLBACK_OPENSSL` | Enable OpenSSL fallback. |
| `KURLYK_USE_FALLBACK_CURL` | Enable libcurl fallback. |
| `KURLYK_USE_FALLBACK_ASIO` | Enable Asio fallback. |
| `KURLYK_USE_FALLBACK_SIMPLE_WS_SERVER` | Enable Simple-WebSocket-Server fallback. |
| `KURLYK_OPENSSL_SHARED` | Load OpenSSL as a shared library when the fallback is enabled. |
| `KURLYK_CURL_SHARED` | Load libcurl as a shared library when the fallback is enabled. |
| `KURLYK_BUILD_EXAMPLES` | Build all targets from the `examples/` directory. |

### Adding kurlyk

Add the path to the kurlyk header files:

```
kurlyk/include
```

**kurlyk** is a header-only library, so you only need to include it with `#include <kurlyk.hpp>` to start using it.

## Initialization specifics

**C++11/14**

When using C++11 or C++14, disable automatic initialization and initialize the library manually:

```cpp
#define KURLYK_AUTO_INIT 0
#include <kurlyk.hpp>

int main() {
    kurlyk::init(true);
    // your network code
    kurlyk::deinit();
}
```

Call `kurlyk::init()` **exactly once** before using the library and `kurlyk::deinit()` **also once** before program termination.

**C++17+ (if automatic initialization enabled)**

Starting from C++17, thread-safe automatic initialization is supported. By default, the library initializes itself automatically, and explicit calls to `kurlyk::init()` and `kurlyk::deinit()` are not required.
Automatic initialization behavior can be controlled via configuration macros: `KURLYK_AUTO_INIT` and `KURLYK_AUTO_INIT_USE_ASYNC` (see [Configuration Macros](#configuration-macros)).

`kurlyk::deinit()` is the normal cleanup call for both asynchronous `init(true)` and synchronous `init(false)` modes. `kurlyk::shutdown()` remains available for explicit manager cleanup/reset scenarios, but normal manual lifetime management should use the `init()` / `deinit()` pair.

## Configuration Macros

Define these macros before including `kurlyk.hpp` to fine-tune the library:

| Macro | Default | Description |
|-------|---------|-------------|
| `KURLYK_AUTO_INIT` | `1` | Automatically registers managers during static initialization. |
| `KURLYK_AUTO_INIT_USE_ASYNC` | `1` | Starts the network worker thread in the background when auto-init is enabled. Set to `0` for manual processing. |
| `KURLYK_HTTP_SUPPORT` | `1` | Enables or disables the HTTP part of the library. |
| `KURLYK_WEBSOCKET_SUPPORT` | `1` | Enables or disables the WebSocket part of the library. |
| `KURLYK_ENABLE_JSON` | `0` | Adds JSON serialization helpers for some types. |
| `KURLYK_USE_JSON` | undefined | Enables enum-to-JSON helpers in `type_utils.hpp`; usually set alongside `KURLYK_ENABLE_JSON`. |

## Repository layout

| Path | Purpose |
|------|---------|
| `include/` | Public header-only library. |
| `include/kurlyk/core` | Core infrastructure with `NetworkWorker` and base interfaces. |
| `include/kurlyk/http` | HTTP client and request management. |
| `include/kurlyk/websocket` | WebSocket client and connection management. |
| `include/kurlyk/types` | Shared enums, cookies, proxy config and helpers. |
| `include/kurlyk/utils` | Encoding, URL, HTTP, path and error helpers. |
| `tests/integration` | Windows dependency and integration build checks. |
| `tests/odr` | Header-only ODR checks. |
| `tests/smoke` | Portable header smoke checks. |
| `examples/` | Usage examples. |

## Tests

Run the Windows integration suite:

```powershell
powershell -ExecutionPolicy Bypass -File tests/integration/run_integration_tests.ps1
```

Run the ODR suite:

```powershell
powershell -ExecutionPolicy Bypass -File tests/odr/run_odr_tests.ps1
```

Build the portable smoke test manually:

```bash
c++ tests/smoke/header_smoke.cpp -Iinclude -std=c++11 -o header_smoke
./header_smoke

c++ tests/smoke/header_smoke.cpp -Iinclude -std=c++17 -o header_smoke
./header_smoke
```

## Documentation

Generate Doxygen documentation:

```bash
doxygen Doxyfile
```

Published documentation is available at <https://newyaroslav.github.io/kurlyk/>.

## License

This library is distributed under the MIT license. See the [LICENSE](LICENSE) file in the repository for details.

## Support

If you have questions or issues using the library, you can refer to the documentation or ask a question in the GitHub Issues section.

In short, **kurlyk!**

![logo](docs/logo-mini-end.png)
