function(load_simple_server target)

	include(FetchContent)
	FetchContent_Declare(simple_server
		GIT_REPOSITORY https://gitlab.com/eidheim/Simple-Web-Server.git
		GIT_TAG 187f798d54a9c6cee742f2eb2c54e9ba26f5a385
	)
	FetchContent_GetProperties(simple_server)
	if (NOT simple_server_POPULATED)
		FetchContent_Populate(simple_server)
	endif()
	message(STATUS "Simple-Web-Server: using fallback from remote repository")
	if (NOT TARGET simple_server)
		add_library(simple_server INTERFACE)
		target_include_directories(simple_server INTERFACE "${simple_server_SOURCE_DIR}")
	endif()
	target_link_libraries(${target} INTERFACE simple_server)
	
	
endfunction()