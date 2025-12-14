cmake_minimum_required(VERSION 3.21)
project(kurlyk LANGUAGES CXX)
# options

option(USE_STANDALONE_ASIO "Use Asio standalone instead of Boost.Asio" ON)

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

# зависимости от винды
if (WIN32)
	include(cmake/deps/win_deps.cmake)
elseif(APPLE)
	message(FATAL_ERROR "This option for MacOS is not supported yet.")
else()
	message(FATAL_ERROR "This option for Linux is not supported yet.")
endif()
use_os_deps(kurlyk)