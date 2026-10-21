cmake_minimum_required(VERSION 3.20)
project(c166_emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(c166_core STATIC
    src/decode.cpp
    src/fetch.cpp
    src/memory.cpp
)
target_include_directories(c166_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(c166_core PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()
