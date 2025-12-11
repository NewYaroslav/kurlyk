# for now only for MinGW
function(use_os_deps out_target)
	target_link_libraries(${out_target} INTERFACE
		ws2_32
		crypt32
		wsock32
	)
endfunction()