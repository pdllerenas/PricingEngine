#include <benchmark/benchmark.h>

#include <vector>

#include "common/matrix.hpp"

constexpr size_t MATRIX_SIZE = 2000;

static void BM_BadRowMajorIteration(benchmark::State& state) {
  ColumnMatrix<double> matrix(MATRIX_SIZE, MATRIX_SIZE);
  std::vector<double> vec(MATRIX_SIZE, 1.5);
  std::vector<double> result(MATRIX_SIZE, 0.0);

  matrix(0, 0) = 1.0;

  for (auto _ : state) {
    std::fill(result.begin(), result.end(), 0.0);

    for (size_t r = 0; r < MATRIX_SIZE; ++r) {
      double sum = 0.0;
      for (size_t c = 0; c < MATRIX_SIZE; ++c) {
        sum += matrix(r, c) * vec[c];
      }
      result[r] = sum;
    }
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_BadRowMajorIteration);

static void BM_GoodColumnMajorIteration(benchmark::State& state) {
  ColumnMatrix<double> matrix(MATRIX_SIZE, MATRIX_SIZE);
  std::vector<double> vec(MATRIX_SIZE, 1.5);
  std::vector<double> result(MATRIX_SIZE, 0.0);

  matrix(0, 0) = 1.0;

  for (auto _ : state) {
    std::fill(result.begin(), result.end(), 0.0);

    for (size_t c = 0; c < MATRIX_SIZE; ++c) {
      const double vec_val = vec[c];
      const double* col_data = matrix.col_ptr(c);

      for (size_t r = 0; r < MATRIX_SIZE; ++r) {
        result[r] += col_data[r] * vec_val;
      }
    }
    benchmark::DoNotOptimize(result);
  }
}
BENCHMARK(BM_GoodColumnMajorIteration);