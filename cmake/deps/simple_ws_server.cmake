function(use_or_fetch_simple_ws_server out_target)
	
	if(TARGET simple_ws_server)
		message(AUTHOR_WARNING "Simple-Websocket-Server: using existing target simple_ws_server")
		if(NOT DEFINED USE_STANDALONE_ASIO)
			set(USE_STANDALONE_ASIO ${KURLYK_USE_STANDALONE_ASIO}
			  CACHE BOOL "Synchronization for Simple-WebSocket-Server" FORCE)
		endif()
		target_link_libraries(${out_target} INTERFACE simple_ws_server)
		return()
	endif()
	
	if(KURLYK_USE_FALLBACK_SIMPLE_WS_SERVER)
		include(cmake/deps/fallbacks/load_simple_ws_server.cmake)
		load_simple_ws_server(${out_target})
		return()
	endif()
	
	message(FATAL_ERROR "Target simple_ws_server not found and fallback option is disabled")
	
endfunction()