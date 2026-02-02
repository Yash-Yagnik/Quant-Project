#include "lumina/market_data_handler.hpp"
#include "lumina/thread_utils.hpp"
#include <chrono>

namespace lumina {

MarketDataHandler::MarketDataHandler(std::shared_ptr<MDRing> to_strategy)
  : to_strategy_(std::move(to_strategy)) {}

MarketDataHandler::~MarketDataHandler() { stop(); }

void MarketDataHandler::start() {
  if (running_.exchange(true)) return;
  thread_ = std::thread(&MarketDataHandler::run, this);
}

void MarketDataHandler::stop() {
  running_.store(false);
  if (thread_.joinable()) thread_.join();
}

void MarketDataHandler::on_trade(Price price, Qty qty, TimestampNs ts_ns) {
  MarketDataEvent ev{};
  ev.flag = MDFlag::Trade;
  ev.ts_ns = ts_ns;
  ev.last_trade.price = price;
  ev.last_trade.qty = qty;
  ev.last_trade.time_ns = ts_ns;
  ev.mid = book_.mid_price();
  ev.bid = book_.best_bid();
  ev.ask = book_.best_ask();
  ev.bid_qty = book_.best_bid_level().total_qty;
  ev.ask_qty = book_.best_ask_level().total_qty;
  book_.get_bid_ask_volumes(ev.bid_volume, ev.ask_volume);
  to_strategy_->try_push(ev);
}

void MarketDataHandler::on_book_update(Side side, Price price, Qty delta_qty, bool is_add) {
  (void)side;
  (void)price;
  (void)delta_qty;
  (void)is_add;
  MarketDataEvent ev{};
  ev.flag = MDFlag::BookUpdate;
  ev.mid = book_.mid_price();
  ev.bid = book_.best_bid();
  ev.ask = book_.best_ask();
  book_.get_bid_ask_volumes(ev.bid_volume, ev.ask_volume);
  to_strategy_->try_push(ev);
}

void MarketDataHandler::run() {
  pin_thread_to_core(1);
  while (running_.load(std::memory_order_acquire)) {
    std::this_thread::sleep_for(std::chrono::microseconds(10));
  }
}

} // namespace lumina
