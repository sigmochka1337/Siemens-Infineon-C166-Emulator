cmake_minimum_required(VERSION 3.20)
project(c166_emulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(c166_contracts INTERFACE)
target_include_directories(c166_contracts INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(c166_contracts INTERFACE cxx_std_20)

function(c166_module target)
    add_library(${target} STATIC ${ARGN})
    target_link_libraries(${target} PUBLIC c166_contracts)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# Each owner can build their module without building the other implementations.
c166_module(c166_memory src/memory/machine.cpp)
c166_module(c166_decoder src/decoder/decoder.cpp src/decoder/fetcher.cpp)
c166_module(c166_executor src/executor/executor.cpp)
c166_module(c166_core src/core/cpu_core.cpp)
c166_module(c166_debugger src/debugger/debugger.cpp)

add_library(c166_emulator INTERFACE)
target_link_libraries(c166_emulator INTERFACE
    c166_memory c166_decoder c166_executor c166_core c166_debugger)

add_executable(c166_demo tools/demo.cpp)
target_link_libraries(c166_demo PRIVATE c166_emulator)

include(CTest)
if(BUILD_TESTING)
    foreach(module memory decoder executor)
        add_executable(test_${module} tests/test_${module}.cpp)
        target_link_libraries(test_${module} PRIVATE c166_${module})
        add_test(NAME ${module} COMMAND test_${module})
    endforeach()
    add_executable(test_integration tests/test_integration.cpp)
    target_link_libraries(test_integration PRIVATE c166_emulator)
    add_test(NAME integration COMMAND test_integration)
    add_test(NAME demo COMMAND c166_demo)

    # Each public header is compiled in its own translation unit.
    file(GLOB public_headers CONFIGURE_DEPENDS
        "src/common/*.h" "src/interfaces/*.h" "src/core/*.h"
        "src/decoder/*.h" "src/executor/*.h" "src/memory/*.h"
        "src/debugger/*.h" "src/emulator/*.h")
    set(header_sources)
    foreach(header IN LISTS public_headers)
        file(RELATIVE_PATH relative_header "${CMAKE_CURRENT_SOURCE_DIR}/src" "${header}")
        string(MAKE_C_IDENTIFIER "${relative_header}" header_id)
        set(header_source "${CMAKE_CURRENT_BINARY_DIR}/header_checks/${header_id}.cpp")
        file(WRITE "${header_source}" "#include \"${relative_header}\"\n")
        list(APPEND header_sources "${header_source}")
    endforeach()
    add_library(c166_header_checks OBJECT ${header_sources})
    target_link_libraries(c166_header_checks PRIVATE c166_contracts)
endif()
