#include "lumina/fix_engine.hpp"
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace lumina {

std::unordered_map<int, std::string> FixEngine::parse_tags(const char* msg, size_t len) {
  std::unordered_map<int, std::string> out;
  const char* p = msg;
  const char* end = msg + len;
  while (p < end) {
    const char* eq = static_cast<const char*>(std::memchr(p, '=', end - p));
    if (!eq) break;
    int tag = static_cast<int>(std::strtol(p, nullptr, 10));
    const char* val_start = eq + 1;
    const char* sep = static_cast<const char*>(std::memchr(val_start, '|', end - val_start));
    if (sep)
      out[tag] = std::string(val_start, sep - val_start);
    else {
      out[tag] = std::string(val_start, end - val_start);
      break;
    }
    p = sep + 1;
  }
  return out;
}

bool FixEngine::parse(const char* msg, size_t len) {
  auto tags = parse_tags(msg, len);
  auto mt = tags.find(fix::MsgType);
  if (mt == tags.end()) return false;
  if (mt->second == "D" || mt->second == "\x44") {
    auto cid = tags.find(fix::ClOrdID);
    auto px = tags.find(fix::Price);
    auto qty = tags.find(fix::OrderQty);
    auto side = tags.find(fix::Side);
    if (cid == tags.end() || px == tags.end() || qty == tags.end() || side == tags.end())
      return false;
    OrderId id = static_cast<OrderId>(std::strtoull(cid->second.c_str(), nullptr, 10));
    Price price = static_cast<Price>(std::strtoll(px->second.c_str(), nullptr, 10));
    Qty order_qty = static_cast<Qty>(std::strtoll(qty->second.c_str(), nullptr, 10));
    Side s = (side->second == "1" || side->second == "B") ? Side::Buy : Side::Sell;
    if (order_callback_) order_callback_(id, price, order_qty, s);
    return true;
  }
  if (mt->second == "F" || mt->second == "\x46") {
    auto cid = tags.find(fix::ClOrdID);
    if (cid != tags.end() && cancel_callback_) {
      OrderId id = static_cast<OrderId>(std::strtoull(cid->second.c_str(), nullptr, 10));
      cancel_callback_(id);
      return true;
    }
  }
  return false;
}

std::string FixEngine::build_new_order_single(OrderId cl_ord_id, const std::string& symbol,
                                              Side side, Qty qty, Price price) {
  std::ostringstream os;
  os << "35=D|11=" << cl_ord_id << "|55=" << symbol << "|54=" << (side == Side::Buy ? "1" : "2")
     << "|40=2|44=" << price << "|38=" << qty << "|59=1|";
  return os.str();
}

std::string FixEngine::build_cancel_request(OrderId cl_ord_id, OrderId orig_cl_ord_id) {
  std::ostringstream os;
  os << "35=F|11=" << cl_ord_id << "|41=" << orig_cl_ord_id << "|";
  return os.str();
}

} // namespace lumina
