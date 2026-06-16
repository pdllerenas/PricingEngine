#include <gtest/gtest.h>

#include <vector>

#include "common/matrix.hpp"
#include "../src/pricing_engine/ms_lbfgs.hpp"

class MsLbfgsTest : public ::testing::Test {
 protected:
  const size_t N = 3;
  const size_t M = 5;

  std::unique_ptr<MsLbfgsSolver> solver;

  void SetUp() override { solver = std::make_unique<MsLbfgsSolver>(N, M); }
};

// Test 1
// If no history, step in steepest direction
TEST_F(MsLbfgsTest, ZeroHistorySteepestDescent) {
  std::vector<double> gradient = {1.5, -2.0, 0.0};
  std::vector<double> direction(N, 0.0);

  solver->compute_search_direction(gradient, direction);

  EXPECT_DOUBLE_EQ(direction[0], -1.5);
  EXPECT_DOUBLE_EQ(direction[1], 2.0);
  EXPECT_DOUBLE_EQ(direction[2], 0.0);
}

TEST_F(MsLbfgsTest, RejectsNegativeCurvature) {
  std::vector<double> step = {1.0, 1.0, 1.0};
  std::vector<double> bad_grad_diff = {-2.0, -2.0, 1.0};

  solver->update_history(step, bad_grad_diff);

  std::vector<double> gradient = {5.0, 5.0, 5.0};
  std::vector<double> direction(N, 0.0);

  solver->compute_search_direction(gradient, direction);

  EXPECT_DOUBLE_EQ(direction[0], -5.0);
  EXPECT_DOUBLE_EQ(direction[1], -5.0);
  EXPECT_DOUBLE_EQ(direction[2], -5.0);
}

TEST_F(MsLbfgsTest, ValidHistoryAltersDirection) {
    // 1. Create a valid step and grad_diff (Positive Curvature)
    std::vector<double> step = {0.1, 0.1, 0.1};
    std::vector<double> grad_diff = {0.5, 0.5, 0.5}; 
    // dot_ys = 0.15 (Valid!)

    solver->update_history(step, grad_diff);

    // 2. Compute a new direction
    std::vector<double> gradient = {1.0, 1.0, 1.0};
    std::vector<double> direction(N, 0.0);
    
    solver->compute_search_direction(gradient, direction);

    // 3. Since history was applied, the direction should NO LONGER be just -gradient.
    // If it is exactly -1.0, our two-loop recursion did absolutely nothing.
    EXPECT_NE(direction[0], -1.0);
    EXPECT_NE(direction[1], -1.0);
    EXPECT_NE(direction[2], -1.0);
    
    // 4. Because our inputs were perfectly symmetric, the output should remain symmetric.
    EXPECT_DOUBLE_EQ(direction[0], direction[1]);
    EXPECT_DOUBLE_EQ(direction[1], direction[2]);
}

// --- TEST 4: The Ring Buffer Overwrite ---
// Push more updates than the maximum history (M=5) to ensure it doesn't crash 
// and gracefully rotates the head pointer.
TEST_F(MsLbfgsTest, SafelyWrapsRingBuffer) {
    std::vector<double> step = {0.1, 0.2, 0.3};
    std::vector<double> grad_diff = {1.0, 2.0, 3.0}; 

    // Push 8 updates (exceeding the limit of 5)
    for (int i = 0; i < 8; ++i) {
        solver->update_history(step, grad_diff);
    }

    std::vector<double> gradient = {0.5, 0.5, 0.5};
    std::vector<double> direction(N, 0.0);
    
    // If the memory rotation is flawed, this will segfault or throw a memory error.
    ASSERT_NO_THROW({
        solver->compute_search_direction(gradient, direction);
    });
}


