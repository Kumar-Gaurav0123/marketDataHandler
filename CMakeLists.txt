cmake_minimum_required(VERSION 3.16)
project(marketDataHandler LANGUAGES CXX)

# ---- Platform check ----
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project targets Linux only (requires epoll, pthread affinity, TSC).")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Default to Release if not set ----
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ---- Collect sources ----
file(GLOB_RECURSE SRC_FILES
    include/**/*.cpp
)

# ---- Create executable ----
add_executable(feed_handler ${SRC_FILES})

# ---- Include directories ----
target_include_directories(feed_handler
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

# ---- Compiler flags ----
target_compile_options(feed_handler
    PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        $<$<CONFIG:Release>:-O3 -march=native -DNDEBUG>
        $<$<CONFIG:Debug>:-g -O0>
)

# ---- Link libraries ----
target_link_libraries(feed_handler pthread)
