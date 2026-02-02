#pragma once

#include <cstddef>
#include <vector>

namespace lumina {

/// SIMD-accelerated indicators (AVX-512 when available).
/// Fallback to scalar if not compiled with AVX-512.

/// Variance over [data, data+len], for volatility estimate.
double variance_simd(const double* data, size_t len);

/// EMA of price series: out[i] = alpha * data[i] + (1-alpha) * out[i-1].
void ema_simd(const double* data, double* out, size_t len, double alpha);

/// Rolling sum (for OBI-style volume sums) - optional SIMD.
double sum_simd(const double* data, size_t len);

} // namespace lumina
