function(use_or_fetch_simple_ws_server out_target)
	
	if(TARGET simple_ws_server)
		message(AUTHOR_WARNING "Simple-Websocket-Server: using existing target simple_server")
		target_link_libraries(${out_target} INTERFACE simple_ws_server)
		return()
	endif()
	
	if(USE_FALLBACK_SIMPLE_WS_SERVER)
		include(cmake/deps/fallbacks/load_simple_ws_server.cmake)
		load_simple_ws_server(${out_target})
		return()
	endif()
	
	message(FATAL_ERROR "Target simple_ws_server not found found and fallback option is disabled")
	
endfunction()