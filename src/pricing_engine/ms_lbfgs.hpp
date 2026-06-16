#pragma once

#include <cassert>
#include <numeric>
#include <vector>

#include "common/matrix.hpp"

class MsLbfgsSolver {
 private:
  const size_t num_params_;   // problem dimension
  const size_t max_history_;  // number of secant equations to consider at most

  ColumnMatrix<double> S_;   // step vectors
  ColumnMatrix<double> Y_;   // gradient diffs
  std::vector<double> rho_;  // scalar array 1.0 / (y^T s)

  size_t head_idx_{0};      // points to the next available column to overwrite
  size_t history_size_{0};  // current number of supported secants

  std::vector<double> q_;
  std::vector<double> r_;
  std::vector<double> alpha_;

 public:
  MsLbfgsSolver(size_t num_params, size_t max_history)
      : num_params_(num_params),
        max_history_(max_history),
        S_(num_params, max_history),
        Y_(num_params, max_history),
        rho_(max_history, 0.0),
        q_(num_params, 0.0),
        r_(num_params, 0.0),
        alpha_(max_history, 0.0) {}

  MsLbfgsSolver(const MsLbfgsSolver&) = delete;
  MsLbfgsSolver& operator=(const MsLbfgsSolver&) = delete;

  void compute_search_direction(const std::vector<double>& gradient,
                                std::vector<double>& search_direction);

  void update_history(const std::vector<double>& step,
                      const std::vector<double>& grad_diff);
};