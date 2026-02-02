#include "lumina/strategy_engine.hpp"
#include <cmath>

namespace lumina {

StrategyEngine::StrategyEngine(std::shared_ptr<MDRing> from_md,
                               double gamma, double sigma, double T_seconds,
                               PreTradeRisk& risk)
  : from_md_(std::move(from_md)), as_(gamma, sigma, T_seconds), obi_(0.1), risk_(risk) {}

void StrategyEngine::poll() {
  MarketDataEvent ev;
  while (from_md_->try_pop(ev)) {
    double s = static_cast<double>(ev.mid);
    double t_sec = (ev.ts_ns - session_start_ns_) / 1e9;
    obi_.update(ev.bid_volume, ev.ask_volume);
    double obi_skew = obi_.value();
    last_r_ = as_.reservation_price(s, t_sec, 0.0);
    double bid_off, ask_off;
    as_.get_quotes(s, t_sec, 0.0, k_, obi_skew, bid_off, ask_off);
    Price bid_price = static_cast<Price>(std::round(bid_off));
    Price ask_price = static_cast<Price>(std::round(ask_off));
    if (order_cb_ && risk_.check_order(bid_price, 100, Side::Buy))
      order_cb_(0, bid_price, 100, Side::Buy, true);
    if (order_cb_ && risk_.check_order(ask_price, 100, Side::Sell))
      order_cb_(0, ask_price, 100, Side::Sell, false);
  }
}

} // namespace lumina
