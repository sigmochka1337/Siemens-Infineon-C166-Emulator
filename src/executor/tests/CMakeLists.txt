cmake_minimum_required(VERSION 3.20)

project(c166_tests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

get_filename_component(C166_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
file(GLOB_RECURSE C166_SOURCES CONFIGURE_DEPENDS "${C166_SOURCE_DIR}/*.cpp")
list(REMOVE_ITEM C166_SOURCES "${C166_SOURCE_DIR}/main.cpp")

add_library(c166_core STATIC ${C166_SOURCES})
target_include_directories(c166_core PUBLIC "${C166_SOURCE_DIR}")

if(MSVC)
    target_compile_options(c166_core PUBLIC /W4 /permissive-)
else()
    target_compile_options(c166_core PUBLIC
        -Wall
        -Wextra
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(c166_core INTERFACE -fsanitize=address,undefined)
endif()

# Use .cc to keep test sources outside the application's recursive *.cpp glob.
file(GLOB instruction_tests CONFIGURE_DEPENDS "instructions/*.cc")

foreach(source IN LISTS instruction_tests)
    get_filename_component(instruction "${source}" NAME_WE)
    set(target "c166_test_${instruction}")

    add_executable("${target}" "${source}")
    target_link_libraries("${target}" PRIVATE c166_core)
    target_include_directories("${target}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME "instruction.${instruction}" COMMAND "${target}")
    set_tests_properties("instruction.${instruction}" PROPERTIES LABELS instruction)
endforeach()
