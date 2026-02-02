#pragma once

#include "lumina/types.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace lumina {

/// Minimal FIX-like tag constants (FIX 4.4 style).
namespace fix {
  constexpr int MsgType = 35;
  constexpr int NewOrderSingle = 0x44;  // 'D'
  constexpr int OrderCancelRequest = 0x46;  // 'F'
  constexpr int ClOrdID = 11;
  constexpr int Symbol = 55;
  constexpr int Side = 54;
  constexpr int OrdType = 40;
  constexpr int Price = 44;
  constexpr int OrderQty = 38;
  constexpr int TimeInForce = 59;
  constexpr int SOH = 1;
}

/// Very basic FIX message parser/builder for order entry.
class FixEngine {
public:
  using FixCallback = std::function<void(OrderId, Price, Qty, Side)>;

  void set_order_callback(FixCallback cb) { order_callback_ = std::move(cb); }
  void set_cancel_callback(std::function<void(OrderId)> cb) { cancel_callback_ = std::move(cb); }

  /// Parse incoming FIX message (e.g. "35=D|11=123|55=AAPL|54=1|40=2|44=15000|38=100|59=1").
  bool parse(const char* msg, size_t len);

  /// Build NewOrderSingle message.
  std::string build_new_order_single(OrderId cl_ord_id, const std::string& symbol,
                                     Side side, Qty qty, Price price);

  /// Build OrderCancelRequest.
  std::string build_cancel_request(OrderId cl_ord_id, OrderId orig_cl_ord_id);

private:
  std::unordered_map<int, std::string> parse_tags(const char* msg, size_t len);
  FixCallback order_callback_;
  std::function<void(OrderId)> cancel_callback_;
};

} // namespace lumina
