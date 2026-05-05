cmake_minimum_required(VERSION 3.21)

set(KURLYK_EXAMPLE_SOURCES
	async_multi_request_example.cpp
	bybit_funding_history_example.cpp
	cancel_http_requests.cpp
	chatgpt_request_example.cpp
	delayed_request_example.cpp
	http_client_future_example.cpp
	http_client_proxy_example.cpp
	http_streaming_example.cpp
	interrupted_requests_example.cpp
	nested_http_requests_example.cpp
	redirect_handling_example.cpp
	simple_http_request_example.cpp
	threaded_request_processing_example.cpp
	utility_functions_example.cpp
	websocket_client_lifecycle_test.cpp
	websocket_echo_example.cpp
	websocket_independent_clients_example.cpp
)

include(${CMAKE_CURRENT_LIST_DIR}/../cmake/copy_runtime_dlls.cmake)

foreach(example_source IN LISTS KURLYK_EXAMPLE_SOURCES)
	get_filename_component(example_target "${example_source}" NAME_WE)
	add_executable(${example_target} ${example_source})
	target_compile_features(${example_target} PRIVATE cxx_std_17)
	target_link_libraries(${example_target} PRIVATE kurlyk)

	if(example_target STREQUAL "chatgpt_request_example")
		target_include_directories(${example_target} PRIVATE
			"${PROJECT_SOURCE_DIR}/libs/json/include"
		)
	endif()

	if(WIN32)
		copy_runtime_dlls(${example_target})
	endif()
endforeach()
