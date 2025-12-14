# odr test
cmake_minimum_required(VERSION 3.21)
project(app_singletons LANGUAGES CXX)

# отследить оПЦИЮ СБОРКИ С АВТОИНИЦИАЛИЗАЦИЕЙ!!!
list(APPEND CMAKE_MODULE_PATH 
	"${CMAKE_CURRENT_LIST_DIR}/../../cmake"
)

set(KURLYK_USE_FALLBACK_OPENSSL ON CACHE BOOL "Use fallback for OpenSSL" FORCE)
set(KURLYK_USE_FALLBACK_CURL ON CACHE BOOL "Use fallback for libcurl" FORCE)
set(KURLYK_USE_FALLBACK_ASIO ON CACHE BOOL "Use fallback for asio" FORCE)
set(KURLYK_USE_FALLBACK_SIMPLE_WS_SERVER ON CACHE BOOL "Use fallback for simple websocket server" FORCE)

set(KURLYK_OPENSSL_SHARED ON CACHE BOOL "Use shared OpenSSL in fallback" FORCE)
set(KURLYK_CURL_SHARED ON CACHE BOOL "Use shared CURL in fallback" FORCE)

add_executable(src_files
	get_singletons_a.cpp
	get_singletons_b.cpp
	get_singletons_main.cpp
)

add_executable(odr_autoinit
  get_autoinit_a.cpp
  get_autoinit_b.cpp
  get_autoinit_main.cpp
)

add_subdirectory(../.. build)

target_compile_features(src_files PUBLIC cxx_std_17)
target_compile_features(odr_autoinit PUBLIC cxx_std_17)
target_compile_definitions(odr_autoinit PRIVATE KURLYK_AUTO_INIT=1)

target_link_libraries(src_files PRIVATE kurlyk)
target_link_libraries(odr_autoinit PRIVATE kurlyk)

include(copy_runtime_dlls)
copy_runtime_dlls(src_files)
