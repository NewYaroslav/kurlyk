function(use_or_fetch_hmac_cpp out_target)

    if(TARGET hmac_cpp::hmac_cpp)
        message(AUTHOR_WARNING "Target hmac_cpp::hmac_cpp was found")
        target_link_libraries(${out_target} INTERFACE hmac_cpp::hmac_cpp)
        return()
    endif()

    find_package(hmac_cpp QUIET)
    if(hmac_cpp_FOUND AND TARGET hmac_cpp::hmac_cpp)
        message(STATUS "hmac-cpp: using existing hmac_cpp::hmac_cpp")
        target_link_libraries(${out_target} INTERFACE hmac_cpp::hmac_cpp)
        return()
    endif()

    include(FetchContent)
    FetchContent_Declare(hmac_cpp
        GIT_REPOSITORY https://github.com/NewYaroslav/hmac-cpp.git
        GIT_TAG 33172abd65520ca5f1a084f5000f0d666f281a97
    )
    FetchContent_MakeAvailable(hmac_cpp)
    if(NOT TARGET hmac_cpp::hmac_cpp)
        message(FATAL_ERROR "hmac-cpp target not available after FetchContent")
    endif()
    target_link_libraries(${out_target} INTERFACE hmac_cpp::hmac_cpp)
    message(STATUS "hmac-cpp: using fallback from remote repository")

endfunction()
