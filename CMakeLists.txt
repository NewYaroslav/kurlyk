cmake_minimum_required(VERSION 3.21)
project(kurlyk VERSION 1.0.2 LANGUAGES CXX)
# options

option(KURLYK_USE_STANDALONE_ASIO "Use Asio standalone instead of Boost.Asio" ON)
option(KURLYK_BUILD_EXAMPLES "Build repository examples" OFF)
option(KURLYK_AUTH_SUPPORT "Enable HTTP authentication helpers (Bearer, API-key)" ON)
option(KURLYK_OAUTH_SUPPORT "Enable OAuth2 PKCE client (requires hmac-cpp)" ON)

option(KURLYK_USE_FALLBACK_OPENSSL "Use fallback for OpenSSL" OFF)
option(KURLYK_USE_FALLBACK_CURL "Use fallback for libcurl" OFF)
option(KURLYK_USE_FALLBACK_ASIO "Use fallback for asio" OFF)
option(KURLYK_USE_FALLBACK_SIMPLE_WS_SERVER "Use fallback for simple websocket server" OFF)
option(KURLYK_OPENSSL_SHARED "For falllback option: OpenSSL shared" ON)
option(KURLYK_CURL_SHARED "For falllback option: libcurl shared" ON)

# Our header-only library
add_library(kurlyk INTERFACE)
target_include_directories(kurlyk INTERFACE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Deps
#openssl
include(cmake/deps/openssl.cmake)
use_or_fetch_openssl(kurlyk)

#curl
include(cmake/deps/curl.cmake)
use_or_fetch_curl(kurlyk)

#asio
include(cmake/deps/asio.cmake)
use_or_fetch_asio(kurlyk)

# simple web-socket server
include(cmake/deps/simple_ws_server.cmake)
use_or_fetch_simple_ws_server(kurlyk)

# Authentication helpers (lightweight — no extra deps)
if(KURLYK_AUTH_SUPPORT)
    target_compile_definitions(kurlyk INTERFACE KURLYK_AUTH_SUPPORT=1)
else()
    target_compile_definitions(kurlyk INTERFACE KURLYK_AUTH_SUPPORT=0)
endif()

if(KURLYK_OAUTH_SUPPORT AND NOT KURLYK_AUTH_SUPPORT)
    message(FATAL_ERROR "KURLYK_OAUTH_SUPPORT requires KURLYK_AUTH_SUPPORT")
endif()

# OAuth2 PKCE (requires hmac-cpp for SHA-256 and CSPRNG)
if(KURLYK_OAUTH_SUPPORT)
    include(cmake/deps/hmac_cpp.cmake)
    use_or_fetch_hmac_cpp(kurlyk)
    target_compile_definitions(kurlyk INTERFACE KURLYK_OAUTH_SUPPORT=1)
else()
    target_compile_definitions(kurlyk INTERFACE KURLYK_OAUTH_SUPPORT=0)
endif()

# OS-specific dependencies
if (WIN32)
	include(cmake/deps/win_deps.cmake)
elseif(UNIX AND NOT APPLE)
	include(cmake/deps/unix_deps.cmake)
elseif(APPLE)
	message(FATAL_ERROR "Full CMake build for macOS is not supported yet.")
else()
	message(FATAL_ERROR "Unsupported platform.")
endif()
use_os_deps(kurlyk)

if(KURLYK_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
