cmake_minimum_required(VERSION 3.20)

project(c166 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(C166_SANITIZE "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

function(c166_target_options target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

if(C166_SANITIZE AND NOT MSVC)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

# Ядро без main.cpp: его переиспользуют CLI и тесты.
set(C166_CORE_SOURCES
    src/fetcher.cpp
    src/flat_machine.cpp
    src/loader.cpp
    src/operands.cpp
    src/types.cpp
)

add_library(c166_core STATIC ${C166_CORE_SOURCES})
target_include_directories(c166_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
c166_target_options(c166_core)

add_executable(c166 src/main.cpp)
target_link_libraries(c166 PRIVATE c166_core)
c166_target_options(c166)

enable_testing()

add_executable(fetcher_test tests/fetcher_test.cpp)
target_link_libraries(fetcher_test PRIVATE c166_core)
c166_target_options(fetcher_test)
add_test(NAME fetcher COMMAND fetcher_test)
