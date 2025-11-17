cmake_minimum_required(VERSION 3.14)
project(kurlyk LANGUAGES CXX)

include(FetchContent)

# Our header-only library
add_library(kurlyk INTERFACE)
target_include_directories(kurlyk INTERFACE 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Deps
set(OPENSSL_USE_STATIC_LIBS TRUE)
find_package(OpenSSL 3.4.0 EXACT REQUIRED)
message(STATUS "Found OpenSSL ${OpenSSL_VERSION}")
find_package(CURL 8.11.0 EXACT REQUIRED)
message(STATUS "Found CURL ${CURL_VERSION_STRING}")

#пока так, потом исправить
find_package(Boost 1.88 COMPONENTS asio)
if (Boost_FOUND)
	target_link_libraries(kurlyk INTERFACE Boost::asio)
	set(USE_STANDALONE_ASIO OFF CACHE BOOL "Use standalone Asio in SWS" FORCE)
else()
	find_package(asio 1.34.2 EXACT REQUIRED)
	set(USE_STANDALONE_ASIO ON CACHE BOOL "Use standalone Asio in SWS" FORCE)
	message(STATUS "Found asio ${asio_VERSION} at: ${asio_DIR}")
	target_link_libraries(kurlyk INTERFACE asio::asio)
endif()

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

target_link_libraries(kurlyk INTERFACE 
	OpenSSL::SSL 
	OpenSSL::Crypto
	CURL::libcurl
	simple-websocket-server
	simple-web-server
	ws2_32
	crypt32
	wsock32
)