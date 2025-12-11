function(load_simple_ws_server target)

	include(FetchContent)
	FetchContent_Declare(simple_ws_server
		GIT_REPOSITORY https://gitlab.com/eidheim/Simple-WebSocket-Server.git
		GIT_TAG 7bb2867b9d50ff559c60b99178fd46531daa2c7e
	)
	FetchContent_GetProperties(simple_ws_server)
	if (NOT simple_ws_server_POPULATED)
		FetchContent_Populate(simple_ws_server)
	endif()
	message(STATUS "Simple-Websocket-Server: using fallback from remote repository")
	if (NOT TARGET simple_ws_server)
		add_library(simple_ws_server INTERFACE)
		target_include_directories(simple_ws_server INTERFACE "${simple_ws_server_SOURCE_DIR}")
	endif()
	target_link_libraries(${target} INTERFACE simple_ws_server)
	
	
endfunction()