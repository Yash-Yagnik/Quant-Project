#include <benchmark/benchmark.h>
#include "lumina/simd_indicators.hpp"
#include <vector>
#include <random>

static void BM_VarianceSimd(benchmark::State& state) {
  size_t n = static_cast<size_t>(state.range(0));
  std::vector<double> data(n);
  std::mt19937 gen(42);
  std::uniform_real_distribution<> dis(99.0, 101.0);
  for (size_t i = 0; i < n; ++i) data[i] = dis(gen);
  for (auto _ : state)
    benchmark::DoNotOptimize(lumina::variance_simd(data.data(), n));
}
BENCHMARK(BM_VarianceSimd)->Arg(256)->Arg(4096)->Arg(65536);

static void BM_SumSimd(benchmark::State& state) {
  size_t n = static_cast<size_t>(state.range(0));
  std::vector<double> data(n, 1.0);
  for (auto _ : state)
    benchmark::DoNotOptimize(lumina::sum_simd(data.data(), n));
}
BENCHMARK(BM_SumSimd)->Arg(256)->Arg(4096)->Arg(65536);
