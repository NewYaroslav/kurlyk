cmake_minimum_required(VERSION 3.14)
project(app_singletons LANGUAGES CXX)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../.. ${CMAKE_BINARY_DIR}/kurlyk EXCLUDE_FROM_ALL)

add_executable(src_files
	get_singletons_a.cpp
	get_singletons_b.cpp
	get_singletons_main.cpp
)

target_link_libraries(src_files PRIVATE kurlyk)

message(STATUS "exe: $<TARGET_FILE:src_files>")