#include "ms_lbfgs.hpp"

void MsLbfgsSolver::compute_search_direction(
    const std::vector<double>& gradient,
    std::vector<double>& search_direction) {
  assert(gradient.size() == num_params_);
  assert(search_direction.size() == num_params_);

  if (history_size_ == 0) {
#pragma omp simd
    for (size_t i = 0; i < num_params_; ++i) {
      search_direction[i] = -gradient[i];
    }
    return;
  }

  std::copy(gradient.begin(), gradient.end(), q_.begin());

  for (size_t i = 0; i < history_size_; ++i) {
    size_t col = (head_idx_ + max_history_ - 1 - i) % max_history_;

    const double* s_col = S_.col_ptr(col);
    const double* y_col = Y_.col_ptr(col);

    // s_i^T * q
    double dot_sq = 0.0;
#pragma omp simd reduction(+ : dot_sq)
    for (size_t j = 0; j < num_params_; ++j) {
      dot_sq += s_col[j] * q_[j];
    }

    alpha_[col] = rho_[col] * dot_sq;

    const double a = alpha_[col];

    // q = q - alpha_i * y_i
#pragma omp simd
    for (size_t j = 0; j < num_params_; ++j) {
      q_[j] -= a * y_col[j];
    }
  }

  // initial hessian approximation
  size_t newest_col = (head_idx_ + max_history_ - 1) % max_history_;
  const double* s_newest = S_.col_ptr(newest_col);
  const double* y_newest = Y_.col_ptr(newest_col);

  double dot_sy = 0.0;
  double dot_yy = 0.0;
#pragma omp simd reduction(+ : dot_sy, dot_yy)
  for (size_t j = 0; j < num_params_; ++j) {
    dot_sy += s_newest[j] * y_newest[j];
    dot_yy += y_newest[j] * y_newest[j];
  }

  double gamma = (dot_yy > 1e-10) ? (dot_sy / dot_yy) : 1.0;

#pragma omp simd
  for (size_t j = 0; j < num_params_; ++j) {
    r_[j] = gamma * q_[j];
  }

  for (size_t i = 0; i < history_size_; ++i) {
    size_t col = (head_idx_ + max_history_ - history_size_ + i) % max_history_;

    const double* s_col = S_.col_ptr(newest_col);
    const double* y_col = Y_.col_ptr(newest_col);

    // y_i^T * r
    double dot_yr = 0.0;
#pragma omp simd reduction(+ : dot_yr)
    for (size_t j = 0; j < num_params_; ++j) {
      dot_yr += y_col[j] * r_[j];
    }

    double beta = rho_[col] * dot_yr;
    const double diff = alpha_[col] - beta;
#pragma omp simd
    for (size_t j = 0; j < num_params_; ++j) {
      r_[j] += s_col[j] * diff;
    }
  }

#pragma omp simd
  for (size_t j = 0; j < num_params_; ++j) {
    search_direction[j] = -r_[j];
  }
}

void MsLbfgsSolver::update_history(const std::vector<double>& step,
                                   const std::vector<double>& grad_diff) {
  assert(step.size() == num_params_);
  assert(grad_diff.size() == num_params_);

  double dot_ys = 0.0;
#pragma omp simd reduction(+ : dot_ys)
  for (size_t i = 0; i < num_params_; ++i) {
    dot_ys += grad_diff[i] * step[i];
  }

  if (dot_ys <= 1e-10) {
    return;
  }

  rho_[head_idx_] = 1.0 / dot_ys;

  double* s_col = S_.col_ptr(head_idx_);
  double* y_col = Y_.col_ptr(head_idx_);

#pragma omp simd
  for (size_t i = 0; i < num_params_; ++i) {
    s_col[i] = step[i];
    y_col[i] = grad_diff[i];
  }

  head_idx_ = (head_idx_ + 1) % max_history_;

  if (history_size_ < max_history_) {
    history_size_++;
  }
}