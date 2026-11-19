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
file(GLOB instruction_entries CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/instructions/*/main.cc"
)

foreach(entry IN LISTS instruction_entries)
    get_filename_component(instruction_dir "${entry}" DIRECTORY)
    get_filename_component(instruction "${instruction_dir}" NAME)
    file(GLOB sources CONFIGURE_DEPENDS "${instruction_dir}/*.cc")
    set(target "c166_test_${instruction}")
    set(test_name "instruction.${instruction}")

    add_executable("${target}" ${sources})
    target_link_libraries("${target}" PRIVATE c166_core)
    target_include_directories("${target}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

    add_test(NAME "${test_name}" COMMAND "${target}")
    set_tests_properties("${test_name}" PROPERTIES LABELS "instruction;${instruction}")
endforeach()
