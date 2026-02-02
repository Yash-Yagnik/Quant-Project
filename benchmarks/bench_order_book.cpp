#include <benchmark/benchmark.h>
#include "lumina/order_book.hpp"

using namespace lumina;

static void BM_OrderBook_AddCancel(benchmark::State& state) {
  OrderBook book(1 << 18);
  OrderId id = 0;
  for (auto _ : state) {
    book.add_order(++id, 10000 + (id % 100), 10, Side::Buy);
    if (id % 2 == 0)
      book.cancel_order(id - 1);
  }
}
BENCHMARK(BM_OrderBook_AddCancel)->Iterations(100000);

static void BM_OrderBook_MidPrice(benchmark::State& state) {
  OrderBook book(1 << 18);
  for (int i = 0; i < 100; ++i) {
    book.add_order(i + 1, 10000 - i, 10, Side::Buy);
    book.add_order(1000 + i + 1, 10010 + i, 10, Side::Sell);
  }
  for (auto _ : state)
    benchmark::DoNotOptimize(book.mid_price());
}
BENCHMARK(BM_OrderBook_MidPrice)->Iterations(1000000);
