#pragma once

#include "lumina/types.hpp"
#include "lumina/order_book.hpp"
#include "lumina/ring_buffer.hpp"
#include <atomic>
#include <cstddef>
#include <memory>
#include <thread>

namespace lumina {

constexpr size_t MD_RING_SIZE = 65536;

/// Consumes raw updates (or simulated ticks), updates order book,
/// and publishes MarketDataEvent to the strategy ring buffer.
class MarketDataHandler {
public:
  using MDRing = SPSCRingBuffer<MarketDataEvent, MD_RING_SIZE>;

  explicit MarketDataHandler(std::shared_ptr<MDRing> to_strategy);
  ~MarketDataHandler();

  void start();
  void stop();
  OrderBook& order_book() { return book_; }
  const OrderBook& order_book() const { return book_; }

  /// Feed a trade (e.g. from exchange or backtester).
  void on_trade(Price price, Qty qty, TimestampNs ts_ns);
  /// Feed book update (add/cancel).
  void on_book_update(Side side, Price price, Qty delta_qty, bool is_add);

private:
  void run();

  std::shared_ptr<MDRing> to_strategy_;
  OrderBook book_;
  std::atomic<bool> running_{false};
  std::thread thread_;
};

} // namespace lumina
