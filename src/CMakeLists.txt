# Create the main executable
add_executable(market_engine ./pricing_engine/ms_lbfgs.cpp main.cpp)

# Link the standard threading library (required for std::jthread)
target_link_libraries(market_engine PRIVATE Threads::Threads OpenMP::OpenMP_CXX)

# Tell the compiler where to find our local header files (like "common/types.hpp")
target_include_directories(market_engine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})