function(load_simple_web_server target)

	if(NOT DEFINED USE_STANDALONE_ASIO)
		set(USE_STANDALONE_ASIO ${KURLYK_USE_STANDALONE_ASIO}
		  CACHE BOOL "Synchronization for Simple-Web-Server" FORCE)
	endif()
	include(FetchContent)
	FetchContent_Declare(simple_web_server
		GIT_REPOSITORY https://gitlab.com/eidheim/Simple-Web-Server.git
		GIT_TAG 35ebb10782507f887802df64a2b6bfc8b427d81f
	)
	FetchContent_GetProperties(simple_web_server)
	if (NOT simple_web_server_POPULATED)
		FetchContent_Populate(simple_web_server)
	endif()
	message(STATUS "Simple-Web-Server: using fallback from remote repository")
	if (NOT TARGET simple_web_server)
		add_library(simple_web_server INTERFACE)
		target_include_directories(simple_web_server INTERFACE "${simple_web_server_SOURCE_DIR}")
	endif()
	target_link_libraries(${target} INTERFACE simple_web_server)

endfunction()
