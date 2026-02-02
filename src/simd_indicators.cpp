#include "lumina/simd_indicators.hpp"
#include <cmath>
#include <numeric>

#if defined(__AVX512F__)
#include <immintrin.h>
#endif

namespace lumina {

#if defined(__AVX512F__)
static inline double hsum_avx512(__m512d v) {
  __m256d lo = _mm512_castpd512_pd256(v);
  __m256d hi = _mm512_extractf64x4_pd(v, 1);
  __m256d sum = _mm256_add_pd(lo, hi);
  __m128d r = _mm256_castpd256_pd128(_mm256_add_pd(sum, _mm256_permute2f128_pd(sum, sum, 1)));
  r = _mm_add_pd(r, _mm_permute_pd(r, 1));
  return _mm_cvtsd_f64(r);
}

static double variance_avx512(const double* data, size_t len) {
  if (len < 8) {
    double sum = 0, sum2 = 0;
    for (size_t i = 0; i < len; ++i) { sum += data[i]; sum2 += data[i] * data[i]; }
    double mean = sum / len;
    return (sum2 / len) - (mean * mean);
  }
  __m512d v_sum = _mm512_setzero_pd();
  __m512d v_sum2 = _mm512_setzero_pd();
  size_t i = 0;
  for (; i + 8 <= len; i += 8) {
    __m512d v = _mm512_loadu_pd(data + i);
    v_sum = _mm512_add_pd(v_sum, v);
#if defined(__FMA__) || defined(__AVX2__)
    v_sum2 = _mm512_fmadd_pd(v, v, v_sum2);
#else
    v_sum2 = _mm512_add_pd(v_sum2, _mm512_mul_pd(v, v));
#endif
  }
  double sum = hsum_avx512(v_sum);
  double sum2 = hsum_avx512(v_sum2);
  for (; i < len; ++i) { sum += data[i]; sum2 += data[i] * data[i]; }
  double mean = sum / len;
  return (sum2 / len) - (mean * mean);
}

static void ema_avx512(const double* data, double* out, size_t len, double alpha) {
  if (len == 0) return;
  out[0] = data[0];
  for (size_t i = 1; i < len; ++i)
    out[i] = alpha * data[i] + (1.0 - alpha) * out[i - 1];
}

static double sum_avx512(const double* data, size_t len) {
  __m512d v_sum = _mm512_setzero_pd();
  size_t i = 0;
  for (; i + 8 <= len; i += 8)
    v_sum = _mm512_add_pd(v_sum, _mm512_loadu_pd(data + i));
  double sum = hsum_avx512(v_sum);
  for (; i < len; ++i) sum += data[i];
  return sum;
}
#endif

double variance_simd(const double* data, size_t len) {
  if (len == 0) return 0.0;
#if defined(__AVX512F__)
  return variance_avx512(data, len);
#else
  double sum = 0, sum2 = 0;
  for (size_t i = 0; i < len; ++i) {
    sum += data[i];
    sum2 += data[i] * data[i];
  }
  double mean = sum / len;
  return (sum2 / len) - (mean * mean);
#endif
}

void ema_simd(const double* data, double* out, size_t len, double alpha) {
  if (len == 0) return;
#if defined(__AVX512F__)
  ema_avx512(data, out, len, alpha);
#else
  out[0] = data[0];
  for (size_t i = 1; i < len; ++i)
    out[i] = alpha * data[i] + (1.0 - alpha) * out[i - 1];
#endif
}

double sum_simd(const double* data, size_t len) {
#if defined(__AVX512F__)
  return sum_avx512(data, len);
#else
  return std::accumulate(data, data + len, 0.0);
#endif
}

} // namespace lumina
