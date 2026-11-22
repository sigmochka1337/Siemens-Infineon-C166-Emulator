cmake_minimum_required(VERSION 3.20)

project(c166 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE C166_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(c166 ${C166_SOURCES})

include(CTest)
set(C166_TARGETS c166)
if(BUILD_TESTING)
    add_executable(c166_fetcher_test
        tests/fetcher_test.cpp
        src/fetcher.cpp
        src/operands.cpp
        src/types.cpp
    )
    add_test(NAME fetcher COMMAND c166_fetcher_test)
    list(APPEND C166_TARGETS c166_fetcher_test)
endif()

foreach(target IN LISTS C166_TARGETS)
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE
            -Wall
            -Wextra
            -fsanitize=address,undefined
            -fno-omit-frame-pointer
        )
        target_link_options(${target} PRIVATE -fsanitize=address,undefined)
    endif()
endforeach()
