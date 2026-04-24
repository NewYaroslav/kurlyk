#integration test
cmake_minimum_required(VERSION 3.21)
project(integration_test LANGUAGES CXX)

#test options
option(KURLYK_TEST_PRELOAD_DEPS "Preload deps before adding kurlyk lib" OFF)

add_executable(integr_test
	get_singletons_a.cpp
	get_singletons_b.cpp
	get_singletons_main.cpp
)

add_executable(ws_local_test
	websocket_local_main.cpp
)

add_executable(backpressure_test
	backpressure_main.cpp
)
list(APPEND CMAKE_MODULE_PATH 
	"${CMAKE_CURRENT_LIST_DIR}/../../cmake/deps"
	"${CMAKE_CURRENT_LIST_DIR}/../../cmake"
)

if(KURLYK_TEST_PRELOAD_DEPS)
message(STATUS "Start preload simple-ws-server, asio, OpenSSl")
#simple-ws-server
	include(fallbacks/load_simple_ws_server)
	load_simple_ws_server(integr_test)
#asio
	include(fallbacks/load_asio)
	load_asio(integr_test)
#openssl
	if (WIN32)
		if(MSVC)
			include(fallbacks/load_openssl_msvc)
		elseif(MINGW)
			include(fallbacks/load_openssl_mingw)
		else()
			message(FATAL_ERROR "Unknown compiler or there is no fallback for this compiler.")
		endif()
	elseif(APPLE)
		message(FATAL_ERROR "OpenSSL fallback option for MacOS is not supported yet")
	else()
		message(FATAL_ERROR "OpenSSL fallback option for Linux is not supported yet")
	endif()
	load_openssl(integr_test)
	
endif()

add_subdirectory(../.. build)

target_link_libraries(integr_test PRIVATE kurlyk)
target_link_libraries(ws_local_test PRIVATE kurlyk)
target_link_libraries(backpressure_test PRIVATE kurlyk)

target_compile_features(integr_test PRIVATE cxx_std_17)
target_compile_features(ws_local_test PRIVATE cxx_std_17)
target_compile_features(backpressure_test PRIVATE cxx_std_17)

if(MINGW)
	target_compile_options(ws_local_test PRIVATE -Wa,-mbig-obj)
endif()

include(copy_runtime_dlls)
copy_runtime_dlls(integr_test)
copy_runtime_dlls(ws_local_test)
copy_runtime_dlls(backpressure_test)
