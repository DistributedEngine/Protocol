# Copyright (c) 2025 — 2026 Ian Torres <iantorres@outlook.com>.
# All rights reserved.

cmake_minimum_required(VERSION 3.25)
project(engine-protocol LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g --coverage -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")

option(BUILD_PROTOCOL_TESTS "Build PROTOCOL tests" ON)
option(BUILD_PROTOCOL_BENCHMARK "Build PROTOCOL benchmark" ON)

add_library(engine-protocol INTERFACE)
target_include_directories(engine-protocol INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_features(engine-protocol INTERFACE cxx_std_23)

include(CMakePackageConfigHelpers)

install(TARGETS engine-protocol
        EXPORT engine-protocolTargets
)

install(EXPORT engine-protocolTargets
        FILE engine-protocolTargets.cmake
        NAMESPACE engine::
        DESTINATION lib/cmake/engine-protocol
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/engine-protocolConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/engine-protocolConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/engine-protocolConfigVersion.cmake"
        DESTINATION lib/cmake/engine-protocol
)

include(FetchContent)

if (BUILD_PROTOCOL_TESTS)
    FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cc)

    add_executable(engine-protocol-tests ${TEST_FILES})

    target_compile_options(engine-protocol-tests PRIVATE --coverage -O0 -g -fprofile-arcs -ftest-coverage)
    target_link_options(engine-protocol-tests PRIVATE --coverage)

    target_link_libraries(engine-protocol-tests
            GTest::gtest_main
            engine-protocol
    )

    include(GoogleTest)
    gtest_discover_tests(engine-protocol-tests)
endif()

if (BUILD_PROTOCOL_BENCHMARK)
    FetchContent_Declare(
            benchmark
            GIT_REPOSITORY https://github.com/google/benchmark.git
            GIT_TAG v1.8.3
    )

    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)

    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(benchmark)

    file(GLOB_RECURSE BENCH_FILES
            CONFIGURE_DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks/*.cc
    )

    add_executable(engine-protocol-benchmarks ${BENCH_FILES})

    target_compile_options(engine-protocol-benchmarks PRIVATE
            -O3 -DNDEBUG
    )

    target_link_libraries(engine-protocol-benchmarks
            benchmark::benchmark_main
            engine-protocol
    )

    target_compile_definitions(engine-protocol-benchmarks PRIVATE BOOST_CONTRACT_NO_CHECKS)
endif ()

add_library(engine::protocol ALIAS engine-protocol)