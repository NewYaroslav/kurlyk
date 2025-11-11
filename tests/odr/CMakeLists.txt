cmake_minimum_required(VERSION 3.14)
project(app_singletons LANGUAGES CXX)

add_library(kurlyk INTERFACE)

add_executable(src_files
	get_singletons_a.cpp
	get_singletons_b.cpp
	get_singletons_main.cpp
)

target_link_libraries(src_files PRIVATE kurlyk)