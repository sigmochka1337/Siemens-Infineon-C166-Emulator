cmake_minimum_required(VERSION 3.20)

project(c166 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE C166_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(c166 ${C166_SOURCES})

target_include_directories(c166
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    target_compile_options(c166 PRIVATE /W4 /permissive-)
else()
    target_compile_options(c166 PRIVATE
        -Wall
        -Wextra
        -fsanitize=address,undefined
        -fno-omit-frame-pointer
    )
    target_link_options(c166 PRIVATE
        -fsanitize=address,undefined
    )
endif()
