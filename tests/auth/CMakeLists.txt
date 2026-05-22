cmake_minimum_required(VERSION 3.21)
project(auth_tests LANGUAGES CXX)

enable_testing()

set(AUTH_TEST_SOURCES
    test_base64url.cpp
    test_pkce.cpp
    test_auth_providers.cpp
    test_oauth_url.cpp
    test_oauth_token_parse.cpp
)

list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_LIST_DIR}/../../cmake/deps"
    "${CMAKE_CURRENT_LIST_DIR}/../../cmake"
)

add_subdirectory(../.. build)

foreach(test_source IN LISTS AUTH_TEST_SOURCES)
    get_filename_component(test_target "${test_source}" NAME_WE)
    add_executable(${test_target} ${test_source})
    target_compile_features(${test_target} PRIVATE cxx_std_17)
    target_link_libraries(${test_target} PRIVATE kurlyk)

    if(WIN32)
        include(copy_runtime_dlls)
        copy_runtime_dlls(${test_target})
    endif()

    add_test(NAME ${test_target} COMMAND ${test_target})
endforeach()
