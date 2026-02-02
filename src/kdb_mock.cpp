#include "lumina/kdb_mock.hpp"
#include <algorithm>

namespace lumina {

void KDBMock::insert(const TickRecord& rec) {
  std::lock_guard<std::mutex> lock(mtx_);
  ticks_.push_back(rec);
}

void KDBMock::insert_trade(TimestampNs ts_ns, Price price, Qty qty) {
  TickRecord rec{};
  rec.ts_ns = ts_ns;
  rec.price = price;
  rec.qty = qty;
  rec.side = Side::Buy;
  insert(rec);
}

std::optional<double> KDBMock::last_price() const {
  std::lock_guard<std::mutex> lock(mtx_);
  if (ticks_.empty()) return std::nullopt;
  return static_cast<double>(ticks_.back().price);
}

std::vector<TickRecord> KDBMock::range(TimestampNs from_ns, TimestampNs to_ns) const {
  std::lock_guard<std::mutex> lock(mtx_);
  std::vector<TickRecord> out;
  for (const auto& t : ticks_) {
    if (t.ts_ns >= from_ns && t.ts_ns <= to_ns)
      out.push_back(t);
  }
  return out;
}

size_t KDBMock::size() const {
  std::lock_guard<std::mutex> lock(mtx_);
  return ticks_.size();
}

} // namespace lumina
