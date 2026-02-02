#pragma once

#include "lumina/types.hpp"
#include "lumina/avellaneda_stoikov.hpp"
#include "lumina/order_book_imbalance.hpp"
#include "lumina/ring_buffer.hpp"
#include "lumina/risk_checks.hpp"
#include "lumina/market_data_handler.hpp"
#include <memory>
#include <atomic>
#include <functional>

namespace lumina {

constexpr size_t STRATEGY_RING_SIZE = 65536;

/// Reads MarketDataEvent from ring, runs Avellaneda-Stoikov + OBI,
/// and emits quote decisions (subject to pre-trade risk).
class StrategyEngine {
public:
  using MDRing = MarketDataHandler::MDRing;

  StrategyEngine(std::shared_ptr<MDRing> from_md,
                 double gamma, double sigma, double T_seconds,
                 PreTradeRisk& risk);

  /// Process one event from the ring (call in tight loop).
  void poll();

  /// Callback when strategy wants to send/cancel orders.
  using OrderCallback = std::function<void(OrderId, Price, Qty, Side, bool is_bid)>;
  using CancelCallback = std::function<void(OrderId)>;
  void set_order_callback(OrderCallback cb) { order_cb_ = std::move(cb); }
  void set_cancel_callback(CancelCallback cb) { cancel_cb_ = std::move(cb); }

  void set_k(double k) { k_ = k; }
  double reservation_price() const { return last_r_; }
  double obi_signal() const { return obi_.value(); }

private:
  std::shared_ptr<MDRing> from_md_;
  AvellanedaStoikov as_;
  OBISignal obi_;
  PreTradeRisk& risk_;
  double k_{1.5};
  double last_r_{0.0};
  double session_start_ns_{0.0};
  OrderCallback order_cb_;
  CancelCallback cancel_cb_;
};

} // namespace lumina
