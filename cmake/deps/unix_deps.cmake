function(use_os_deps out_target)
	find_package(Threads REQUIRED)
	target_link_libraries(${out_target} INTERFACE Threads::Threads)

	if(UNIX AND NOT APPLE)
		target_link_libraries(${out_target} INTERFACE dl)
	endif()
endfunction()
