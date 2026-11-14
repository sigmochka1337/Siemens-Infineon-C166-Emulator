cmake_minimum_required(VERSION 3.20)

project(c166_memory_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_library(c166_memory STATIC ../machine.cpp ../raw_memory.cpp)
target_include_directories(c166_memory PUBLIC "../..")

foreach(test IN ITEMS raw_memory_access machine_access)
    add_executable("${test}" "${test}.cc")
    target_link_libraries("${test}" PRIVATE c166_memory)
    add_test(NAME "memory.${test}" COMMAND "${test}")
endforeach()
