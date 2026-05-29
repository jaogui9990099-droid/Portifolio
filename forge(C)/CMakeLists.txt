cmake_minimum_required(VERSION 3.18)
project(forge VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(FORGE_BUILD_TESTS      "Build the test suite"   ON)
option(FORGE_BUILD_BENCHMARKS "Build benchmarks"       OFF)
option(FORGE_ENABLE_ASAN      "Enable AddressSanitizer" OFF)
option(FORGE_DEBUG_FILL       "Fill freed memory with 0xDF in debug builds" ON)

add_compile_options(
    -Wall -Wextra -Wpedantic
    -Wmissing-prototypes -Wstrict-prototypes
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-O0>
)

if(FORGE_DEBUG_FILL AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(FORGE_DEBUG_FILL=1)
endif()

if(FORGE_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Core library
add_library(forge STATIC
    src/arena.c
    src/pool.c
    src/slab.c
    src/forge.c
)

add_library(forge::forge ALIAS forge)

target_include_directories(forge PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

find_library(MATH_LIB m)
if(MATH_LIB)
    target_link_libraries(forge PUBLIC ${MATH_LIB})
endif()

# Example executable
add_executable(forge-demo src/demo.c)
target_link_libraries(forge-demo PRIVATE forge::forge)

# Tests
if(FORGE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Install
include(GNUInstallDirs)
install(TARGETS forge EXPORT forgeTargets ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
