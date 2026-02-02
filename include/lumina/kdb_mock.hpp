#pragma once

#include "lumina/types.hpp"
#include <vector>
#include <mutex>
#include <optional>

namespace lumina {

/// Mock KDB+/q style time-series storage for tick data.
/// Real KDB+ would use native tables and q language; this is in-memory for testing.
struct TickRecord {
  TimestampNs ts_ns{0};
  Price price{0};
  Qty qty{0};
  Side side{Side::Buy};
  char symbol[16]{0};
};

class KDBMock {
public:
  KDBMock() = default;

  void insert(const TickRecord& rec);
  void insert_trade(TimestampNs ts_ns, Price price, Qty qty);

  std::optional<double> last_price() const;
  std::vector<TickRecord> range(TimestampNs from_ns, TimestampNs to_ns) const;
  size_t size() const;

private:
  mutable std::mutex mtx_;
  std::vector<TickRecord> ticks_;
};

} // namespace lumina
