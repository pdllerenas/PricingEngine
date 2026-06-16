#pragma once
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

template <typename T>
class ColumnMatrix {
 private:
  size_t rows_;
  size_t cols_;

  std::vector<T> data_;

 public:
  ColumnMatrix(size_t rows, size_t cols)
      : rows_(rows), cols_(cols), data_(rows * cols, T{0}) {}

  inline T& operator()(size_t row, size_t col) noexcept {
    return data_[col * rows_ + row];
  }
  inline const T& operator()(size_t row, size_t col) const noexcept {
    return data_[col * rows_ + row];
  }

  inline T* col_ptr(size_t col) noexcept { return &data_[col * rows_]; }
  inline const T* col_ptr(size_t col) const noexcept {
    return &data_[col * rows_];
  }

  [[nodiscard]] size_t rows() const noexcept { return rows_; }
  [[nodiscard]] size_t cols() const noexcept { return cols_; }
};

template <typename T>
void multiply(const ColumnMatrix<T>& matrix, const std::vector<T>& vec,
              std::vector<T>& result) {
  assert(matrix.cols() == vec.size());
  assert(matrix.rows() == result.size());

  std::fill(result.begin(), result.end(), T{0});

  const size_t cols = matrix.cols();
  const size_t rows = matrix.rows();

  for (size_t c = 0; c < cols; ++c) {
    const T vec_val = vec[c];
    const T* col_data = matrix.col_ptr(c);

// vectorize this inner loop (AVX instructions)
#pragma omp simd
    for (size_t r = 0; r < rows; ++r) {
      result[r] += col_data[r] * vec_val;
    }
  }
}