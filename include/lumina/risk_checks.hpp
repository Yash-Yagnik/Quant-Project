#pragma once

#include "lumina/types.hpp"
#include <atomic>
#include <cstdint>
#include <functional>
#include <string>

namespace lumina {

/// Pre-trade risk: kill switch if notional or size exceeds limits.
class PreTradeRisk {
public:
  using NotionalLimit = int64_t;  // max absolute notional (e.g. USD * 100)
  using FatFingerQty = Qty;

  PreTradeRisk(NotionalLimit max_notional, FatFingerQty max_order_qty)
    : max_notional_(max_notional), max_order_qty_(max_order_qty),
      total_notional_(0), is_killed_(false) {}

  /// Returns true if order is allowed, false if risk would be breached.
  bool check_order(Price price, Qty qty, Side side) const {
    if (is_killed_.load(std::memory_order_acquire))
      return false;
    if (qty <= 0 || qty > max_order_qty_)
      return false;
    int64_t notional = static_cast<int64_t>(price) * qty;
    if (notional < 0) notional = -notional;
    if (total_notional_.load(std::memory_order_acquire) + notional > max_notional_)
      return false;
    return true;
  }

  void add_fill(Price price, Qty qty) {
    int64_t notional = static_cast<int64_t>(price) * qty;
    total_notional_.fetch_add(notional < 0 ? -notional : notional,
                              std::memory_order_acq_rel);
  }

  void kill() { is_killed_.store(true, std::memory_order_release); }
  bool killed() const { return is_killed_.load(std::memory_order_acquire); }
  void reset_kill() { is_killed_.store(false, std::memory_order_release); }
  void reset_notional() { total_notional_.store(0, std::memory_order_release); }

private:
  NotionalLimit max_notional_;
  FatFingerQty max_order_qty_;
  std::atomic<int64_t> total_notional_;
  std::atomic<bool> is_killed_;
};

} // namespace lumina
