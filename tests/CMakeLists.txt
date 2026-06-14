# Create the test executable
add_executable(test_ring_buffer test_ring_buffer.cpp)

# Link GoogleTest and the threading library
target_link_libraries(test_ring_buffer PRIVATE GTest::gtest_main Threads::Threads)

# Allow the test file to see the headers inside the src/ directory
target_include_directories(test_ring_buffer PRIVATE ${CMAKE_SOURCE_DIR}/src)

# --- THE FIX IS HERE ---
# Load the GoogleTest CMake module so it recognizes the discovery command
include(GoogleTest)

# Register the executable with CTest
gtest_discover_tests(test_ring_buffer)