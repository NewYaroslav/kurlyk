cmake_minimum_required(VERSION 3.21)
project(kurlyk LANGUAGES CXX)
# options

option(USE_STANDALONE_ASIO "Use Asio standalone instead of Boost.Asio" ON)

option(USE_FALLBACK_OPENSSL "Use fallback for OpenSSL" OFF)
option(USE_FALLBACK_CURL "Use fallback for libcurl" OFF)
option(USE_FALLBACK_ASIO "Use fallback for asio" OFF)
option(USE_FALLBACK_SIMPLE_SERVER "Use fallback for simple web server" OFF)
option(USE_FALLBACK_SIMPLE_WS_SERVER "Use fallback for simple websocket server" OFF)
option(OPENSSL_SHARED "For falllback option: OpenSSL shared" ON)
option(CURL_SHARED "For falllback option: libcurl shared" ON)

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

if (DEFINED USE_OPENSSL AND NOT USE_OPENSSL)
    message(FATAL_ERROR
        "kurlyk requires Simple-Web-Server to be built with OpenSSL (USE_OPENSSL=ON), but the variable USE_OPENSSL is currently OFF!"
    )
endif()

if (NOT DEFINED USE_OPENSSL)
    set(USE_OPENSSL ON CACHE BOOL "Use OpenSSL in Simple-Web-Server")
endif()

# simple web
include(cmake/deps/simple_server.cmake)
use_or_fetch_simple_server(kurlyk)

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