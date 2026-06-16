cmake_minimum_required(VERSION 3.20)

# Project Definition
project(MarketEngine 
        VERSION 0.1.0 
        DESCRIPTION "Low-Latency Feed Handler and Pricing Engine"
        LANGUAGES CXX)

# --- 1. Language Standards ---
# Enforce strictly C++20 (or C++23 if your compiler supports the latest features)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Disable compiler-specific extensions (e.g., gnu++20) to ensure strict portability
set(CMAKE_CXX_EXTENSIONS OFF) 

# --- 2. Compiler Warnings & Optimizations ---
# These flags force you to write clean, deterministic code
set(COMMON_WARNINGS "-Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow")

# Hardware sympathy: -march=native tells the compiler to optimize specifically 
# for the CPU architecture it is building on (crucial for cache-line alignment and vectorization).
set(COMMON_CXX_FLAGS "${COMMON_WARNINGS} -march=native")

# Set up specific profiles for Debug and Release builds
# Release: Max optimization (-O3) and Link-Time Optimization (-flto)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -flto -DNDEBUG ${COMMON_CXX_FLAGS}")
# Debug: Include Address and Undefined Behavior Sanitizers to catch memory leaks and race conditions
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -fsanitize=address,undefined ${COMMON_CXX_FLAGS}")

# --- 3. Core Dependencies ---
# Threading library is mandatory for atomics, mutexes, and your ring buffer
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# OpenMP for parallelizing the MS-LBFGS matrix computations later
find_package(OpenMP REQUIRED)

# --- 4. Testing Framework (GoogleTest) ---
# FetchContent is the cleanest way to pull in external libraries without messing with system packages
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Enable CTest functionality
enable_testing()

# --- 5. Micro-Benchmarking Framework ---
# --- Micro-Benchmarking Framework ---
FetchContent_Declare(
  googlebenchmark
  URL https://github.com/google/benchmark/archive/v1.8.3.zip
)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googlebenchmark)

if(TARGET benchmark)
    target_compile_options(benchmark PRIVATE "-Wno-error" "-Wno-conversion")
endif()
if(TARGET benchmark_main)
    target_compile_options(benchmark_main PRIVATE "-Wno-error" "-Wno-conversion")
endif()

# --- 6. Add Subdirectories ---
add_subdirectory(src)
add_subdirectory(tests)
# add_subdirectory(tools)