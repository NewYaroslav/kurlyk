cmake_minimum_required(VERSION 3.14)
include(FetchContent)

project(kurlyk LANGUAGES CXX)

# Пути к сабмодулям на гитхабе
set(CURL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/curl-8.11.0_1-win64-mingw" CACHE PATH "")
set(OPENSSL_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openssl-win64-v3.4.0" CACHE PATH "")
set(ASIO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio" CACHE PATH "")

# Делаем нашу либу таргетом
add_library(kurlyk INTERFACE)
target_include_directories(kurlyk INTERFACE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# libcurl
add_library(libcurl_static STATIC IMPORTED GLOBAL)
set_target_properties(libcurl_static PROPERTIES 
	IMPORTED_LOCATION "${CURL_ROOT}/lib/libcurl.a"
	INTERFACE_INCLUDE_DIRECTORIES "${CURL_ROOT}/include"
)
target_compile_definitions(libcurl_static INTERFACE CURL_STATICLIB)

# openssl
add_library(openssl_common INTERFACE)
target_include_directories(openssl_common INTERFACE "${OPENSSL_ROOT}/include")

add_library(openssl_capi STATIC IMPORTED GLOBAL)
set_target_properties(openssl_capi PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/capi.lib")

add_library(openssl_dasync STATIC IMPORTED GLOBAL)
set_target_properties(openssl_dasync PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/dasync.lib")
	
add_library(openssl_libcrypto STATIC IMPORTED GLOBAL)
set_target_properties(openssl_libcrypto PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/libcrypto.lib")
	
add_library(openssl_libssl STATIC IMPORTED GLOBAL)
set_target_properties(openssl_libssl PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/libssl.lib")
	
add_library(openssl_openssl STATIC IMPORTED GLOBAL)
set_target_properties(openssl_openssl PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/openssl.lib")
	
add_library(openssl_ossltest STATIC IMPORTED GLOBAL)
set_target_properties(openssl_ossltest PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/ossltest.lib")
	
add_library(openssl_padlock STATIC IMPORTED GLOBAL)
set_target_properties(openssl_padlock PROPERTIES IMPORTED_LOCATION 
	"${OPENSSL_ROOT}/lib/VC/x64/MD/padlock.lib")
	
target_link_libraries(openssl_capi INTERFACE openssl_common)
target_link_libraries(openssl_dasync INTERFACE openssl_common)
target_link_libraries(openssl_libcrypto INTERFACE openssl_common)
target_link_libraries(openssl_libssl INTERFACE openssl_common)
target_link_libraries(openssl_openssl INTERFACE openssl_common)
target_link_libraries(openssl_ossltest INTERFACE openssl_common)
target_link_libraries(openssl_padlock INTERFACE openssl_common)

#asio
add_library(asio INTERFACE)
target_include_directories(asio INTERFACE "${ASIO_ROOT}/asio/include")
target_compile_definitions(asio INTERFACE ASIO_STANDALONE)

#web server and websocket server
set(USE_STANDALONE_ASIO ON CACHE BOOL "Use standalone Asio in SWS" FORCE)

FetchContent_Declare(simple_ws_server
	GIT_REPOSITORY https://gitlab.com/eidheim/Simple-WebSocket-Server.git
	GIT_TAG 7bb2867b9d50ff559c60b99178fd46531daa2c7e
)
FetchContent_MakeAvailable(simple_ws_server)

FetchContent_Declare(simple_server
	GIT_REPOSITORY https://gitlab.com/eidheim/Simple-Web-Server.git
	GIT_TAG 187f798d54a9c6cee742f2eb2c54e9ba26f5a385
)
FetchContent_MakeAvailable(simple_server)

# линкуем к либе
target_link_libraries(kurlyk INTERFACE
	libcurl_static
    openssl_capi
    openssl_dasync
    openssl_libcrypto
	openssl_libssl
	openssl_openssl
	openssl_ossltest
	openssl_padlock
	asio
	simple-websocket-server
	simple-web-server
	ws2_32
	crypt32
	wsock32
)