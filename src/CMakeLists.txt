# Create the main executable
add_executable(market_engine main.cpp)

# Link the standard threading library (required for std::jthread)
target_link_libraries(market_engine PRIVATE Threads::Threads)

# Tell the compiler where to find our local header files (like "common/types.hpp")
target_include_directories(market_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})