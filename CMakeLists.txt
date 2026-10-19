cmake_minimum_required(VERSION 3.20)
project(c166_emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(c166_core STATIC
    src/fetch.cpp
    src/decode.cpp
)
target_include_directories(c166_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(c166_core PRIVATE /W4 /permissive-)
else()
    target_compile_options(c166_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

enable_testing()

add_executable(test_decode tests/test_decode.cpp)
target_link_libraries(test_decode PRIVATE c166_core)
add_test(NAME decode COMMAND test_decode)
