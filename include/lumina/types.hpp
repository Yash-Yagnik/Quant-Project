#pragma once

#include <cstdint>
#include <cstddef>

namespace lumina {

using Price = int64_t;   // fixed-point or integer cents
using Qty = int64_t;
using OrderId = uint64_t;
using TimestampNs = int64_t;

enum class Side : uint8_t { Buy, Sell };

struct Order {
  OrderId id{0};
  Price price{0};
  Qty qty{0};
  Side side{Side::Buy};
  TimestampNs created_ns{0};
  void* pool_node{nullptr};  // for memory pool reuse
};

struct Trade {
  OrderId bid_id{0};
  OrderId ask_id{0};
  Price price{0};
  Qty qty{0};
  TimestampNs time_ns{0};
};

struct BookLevel {
  Price price{0};
  Qty total_qty{0};
  int count{0};
};

// Market data event for ring buffer
enum class MDFlag : uint8_t {
  None,
  Trade,
  BookUpdate,
  BestBidAsk,
};

struct MarketDataEvent {
  MDFlag flag{MDFlag::None};
  TimestampNs ts_ns{0};
  Price mid{0};
  Price bid{0};
  Price ask{0};
  Qty bid_qty{0};
  Qty ask_qty{0};
  Qty bid_volume{0};  // total volume on bid side (for OBI)
  Qty ask_volume{0};  // total volume on ask side
  Trade last_trade{};
};

} // namespace lumina
