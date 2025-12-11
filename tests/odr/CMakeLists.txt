cmake_minimum_required(VERSION 3.21)
project(app_singletons LANGUAGES CXX)

add_executable(src_files
	get_singletons_a.cpp
	get_singletons_b.cpp
	get_singletons_main.cpp
)

set(USE_FALLBACK_OPENSSL ON CACHE BOOL "Use fallback for OpenSSL" FORCE)
set(USE_FALLBACK_CURL ON CACHE BOOL "Use fallback for libcurl" FORCE)
set(USE_FALLBACK_ASIO ON CACHE BOOL "Use fallback for asio" FORCE)
set(USE_FALLBACK_SIMPLE_SERVER ON CACHE BOOL "Use fallback for simple web server" FORCE)
set(USE_FALLBACK_SIMPLE_WS_SERVER ON CACHE BOOL "Use fallback for simple websocket server" FORCE)

set(OPENSSL_SHARED ON CACHE BOOL "Use shared OpenSSL in fallback" FORCE)
set(CURL_SHARED ON CACHE BOOL "Use shared CURL in fallback" FORCE)

add_subdirectory(libs/kurlyk)

# временно
target_compile_features(src_files PUBLIC cxx_std_17)

target_link_libraries(src_files PRIVATE kurlyk)

include(cmake/copy_runtime_dlls.cmake)
copy_runtime_dlls(src_files)
