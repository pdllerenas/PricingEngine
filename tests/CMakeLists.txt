# --- Ring Buffer Test ---
add_executable(test_ring_buffer test_ring_buffer.cpp)
target_link_libraries(test_ring_buffer PRIVATE GTest::gtest_main Threads::Threads OpenMP::OpenMP_CXX)
target_include_directories(test_ring_buffer PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(test_ring_buffer)

# --- Benchmarks ---
add_executable(math_benchmarks benchmark_matrix.cpp)
target_link_libraries(math_benchmarks PRIVATE benchmark::benchmark benchmark::benchmark_main Threads::Threads OpenMP::OpenMP_CXX)
target_include_directories(math_benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/src)

# --- MS-LBFGS Solver Test ---
add_executable(test_ms_lbfgs test_ms_lbfgs.cpp ../src/pricing_engine/ms_lbfgs.cpp)

# Link GTest, Threading, and OpenMP (critical for the SIMD pragmas)
target_link_libraries(test_ms_lbfgs PRIVATE GTest::gtest_main Threads::Threads OpenMP::OpenMP_CXX)

# Allow the test file to see the headers inside the src/ directory
target_include_directories(test_ms_lbfgs PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Register the executable with CTest
gtest_discover_tests(test_ms_lbfgs)