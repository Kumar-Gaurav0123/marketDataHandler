cmake_minimum_required(VERSION 3.16)
project(marketDataHandler LANGUAGES CXX)

# ---- Platform check ----
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project targets Linux only (requires epoll, pthread affinity, TSC).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- Core library (all sources except main) ----
file(GLOB_RECURSE LIB_SOURCES
    src/app/*.cpp
    src/core/*.cpp
    src/network/*.cpp
)

add_library(mdh_lib STATIC ${LIB_SOURCES})

target_include_directories(mdh_lib
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_compile_options(mdh_lib
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
        $<$<CONFIG:Debug>:-g -O0>
)

target_link_libraries(mdh_lib pthread)

# ---- Executable ----
add_executable(feed_handler src/main.cpp)
target_link_libraries(feed_handler mdh_lib)

# ---- Tests ----
option(BUILD_TESTS "Build unit tests" ON)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG        v2.4.11
    )
    FetchContent_MakeAvailable(doctest)

    add_executable(run_tests
        tests/test_fix_parser.cpp
        tests/test_ring_buffer.cpp
    )

    target_link_libraries(run_tests mdh_lib doctest::doctest)

    target_compile_options(run_tests
        PRIVATE
            -Wall -Wextra
            $<$<CONFIG:Debug>:-g -O0>
    )

    enable_testing()
    add_test(NAME unit_tests COMMAND run_tests)
endif()
