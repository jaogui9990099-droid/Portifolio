cmake_minimum_required(VERSION 3.20)
project(nexus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(NEXUS_BUILD_TESTS    "Build the test suite"   ON)
option(NEXUS_BUILD_BENCHMARKS "Build benchmarks"     OFF)
option(NEXUS_ENABLE_ASAN    "Enable AddressSanitizer" OFF)
option(NEXUS_ENABLE_TSAN    "Enable ThreadSanitizer"  OFF)

add_compile_options(
    -Wall -Wextra -Wpedantic -Wshadow -Wcast-align -Wformat=2
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-march=native>
    $<$<CONFIG:Release>:-DNDEBUG>
    $<$<CONFIG:Debug>:-g3>
    $<$<CONFIG:Debug>:-O0>
)

if(NEXUS_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

if(NEXUS_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options(-fsanitize=thread)
endif()

add_library(nexus INTERFACE)
add_library(nexus::nexus ALIAS nexus)

target_include_directories(nexus INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(nexus INTERFACE cxx_std_20)

find_package(Threads REQUIRED)

add_executable(nexus-server src/main.cpp src/server.cpp)
target_link_libraries(nexus-server PRIVATE nexus::nexus Threads::Threads)

if(NEXUS_BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    add_subdirectory(tests)
endif()

include(GNUInstallDirs)
install(TARGETS nexus EXPORT nexusTargets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
