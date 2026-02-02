#include <benchmark/benchmark.h>
#include "lumina/ring_buffer.hpp"
#include "lumina/types.hpp"

using namespace lumina;

static void BM_SPSC_PushPop(benchmark::State& state) {
  SPSCRingBuffer<MarketDataEvent, 65536> rb;
  MarketDataEvent e{};
  e.mid = 10000;
  for (auto _ : state) {
    rb.try_push(e);
    rb.try_pop(e);
  }
}
BENCHMARK(BM_SPSC_PushPop)->Iterations(1000000);
