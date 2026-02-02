#pragma once

#include "lumina/types.hpp"
#include "lumina/memory_pool.hpp"
#include <vector>
#include <unordered_map>
#include <atomic>

namespace lumina {

/// Price level: doubly-linked list of orders for time priority.
struct PriceLevel {
  Price price{0};
  Qty total_qty{0};
  OrderNodePool::Node* head{nullptr};
  OrderNodePool::Node* tail{nullptr};
  PriceLevel* prev{nullptr};
  PriceLevel* next{nullptr};
};

/// Limit Order Book with O(1) price-level lookup via hash map and
/// doubly-linked price levels. All allocations from pool.
class OrderBook {
public:
  explicit OrderBook(size_t max_orders = 1 << 20);
  ~OrderBook() = default;

  bool add_order(OrderId id, Price price, Qty qty, Side side);
  void cancel_order(OrderId id);
  void cancel_order(OrderId id, Price price, Side side);

  /// Match incoming aggressive order (simplified: fill at price levels).
  void match(Side side, Qty qty, std::vector<Trade>& fills);

  Price best_bid() const;
  Price best_ask() const;
  Qty bid_volume() const;  // total volume on bid side (for OBI)
  Qty ask_volume() const;  // total volume on ask side
  Price mid_price() const;
  BookLevel best_bid_level() const;
  BookLevel best_ask_level() const;

  /// Fill depth for OBI / analytics
  void get_bid_ask_volumes(Qty& bid_vol, Qty& ask_vol) const;

private:
  using PriceToLevel = std::unordered_map<Price, PriceLevel*>;
  OrderNodePool pool_;
  std::vector<PriceLevel> level_storage_;
  PriceToLevel bid_levels_;
  PriceToLevel ask_levels_;
  std::unordered_map<OrderId, OrderNodePool::Node*> order_index_;
  PriceLevel* best_bid_{nullptr};
  PriceLevel* best_ask_{nullptr};
  mutable std::atomic<Price> cached_mid_{0};

  PriceLevel* get_or_create_level(Price price, Side side);
  void remove_level_if_empty(PriceLevel* level, Side side);
  void update_best(Side side);
};

} // namespace lumina
